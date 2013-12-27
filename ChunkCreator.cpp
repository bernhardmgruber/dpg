#include <noise/noise.h>
#include <unordered_map>
#include <mutex>

#include "ChunkCreator.h"
#include "globals.h"

#include "tables.inc"

using namespace std;

const unsigned int CHUNK_TRIANGLE_MAP_INITIAL_SIZE = 3000;

ChunkCreator::ChunkCreator(unsigned int loaderThreads)
{
    // create pool threads
    for (unsigned int i = 0; i < loaderThreads; i++)
        loaderThreadPool.push_back(thread(&ChunkCreator::loaderThreadMain, this));
}

ChunkCreator::~ChunkCreator()
{
    // join
    {
        unique_lock<mutex> lock(loadedChunksMutex);
        shutdown = true;
        loadingChunkCV.notify_all();
    }

    for (thread& t : loaderThreadPool)
        t.join();

    for (auto& pair : loadedChunks)
        delete pair.second;
}

Chunk* ChunkCreator::get(const Vector3I& chunkPos)
{
    {
        lock_guard<mutex> lock(loadedChunksMutex);

        // try to find in loaded chunks
        auto it = loadedChunks.find(chunkPos);
        if (it != loadedChunks.end())
        {
            Chunk* c = it->second;
            loadedChunks.erase(it);

            // init chunk before returning it
            c->createBuffers();
            return c;
        }

        // add to chunks which should be loaded if it has not already been added
        if (enqueuedChunksSet.find(chunkPos) == enqueuedChunksSet.end())
        {
            enqueuedChunksQueue.push(chunkPos);
            enqueuedChunksSet.insert(chunkPos);
            loadingChunkCV.notify_one();
        }

        return nullptr;
    }
}

void ChunkCreator::loaderThreadMain()
{
    // influenced by: http://progsch.net/wordpress/?p=81
    while (true)
    {
        Vector3I chunkPos;

        // wait for
        {
            unique_lock<mutex> lock(loadedChunksMutex);

            while (enqueuedChunksSet.size() == 0 && !shutdown)
            {
                cout << "Loader thread #" << std::this_thread::get_id() << " is ready" << endl;
                loadingChunkCV.wait(lock);
                cout << "Loader thread #" << std::this_thread::get_id() << " notified" << endl;
            }

            // check for shutdown
            if (shutdown)
            {
                cout << "Loader thread #" << std::this_thread::get_id() << " exited" << endl;
                return;
            }

            // dequeue load request
            chunkPos = enqueuedChunksQueue.front();
            enqueuedChunksQueue.pop();
            cout << "Loader thread #" << std::this_thread::get_id() << " starts loading chunk " << chunkPos << endl;
        }

        // do load
        Chunk* c = createChunk(chunkPos);

        {
            lock_guard<mutex> lock(loadedChunksMutex);
            loadedChunks[c->getChunkGridPositon()] = c;
            enqueuedChunksSet.erase(enqueuedChunksSet.find(chunkPos));
            cout << "Loader thread #" << std::this_thread::get_id() << " finished loading chunk " << chunkPos << endl;
        }
    }
}

Chunk* ChunkCreator::createChunk(Vector3I chunkGridPosition)
{
    Chunk* c = new Chunk(chunkGridPosition);

    noise::module::Perlin perlin;
    perlin.SetOctaveCount(5);
    perlin.SetFrequency(0.3);

    const unsigned int size = Chunk::RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging
    c->densities = new Chunk::DensityType[size * size * size];

    //Timer timer;

    for (unsigned int x = 0; x < size; x++)
    {
        for (unsigned int y = 0; y < size; y++)
        {
            for (unsigned int z = 0; z < size; z++)
            {
                Vector3F world = c->toWorld(x, y, z);
                c->densities[x * size * size + y * size + z] = (Chunk::DensityType)perlin.GetValue(world.x, world.y, world.z);
            }
        }
    }

    //timer.tick();
    //cout << "Noise took " << timer.interval << " seconds" << endl;

    // create geometry using marching cubes
    marchChunk(c);

    //timer.tick();
    //cout << "Marching took " << timer.interval << " seconds" << endl;

    return c;
}

void ChunkCreator::marchChunk(Chunk* c)
{
    unordered_map<Vector3F, unsigned int> vertexMap(CHUNK_TRIANGLE_MAP_INITIAL_SIZE);

    for (unsigned int x = 1; x < Chunk::RESOLUTION + 1; x++)
    {
        for (unsigned int y = 1; y < Chunk::RESOLUTION + 1; y++)
        {
            for (unsigned int z = 1; z < Chunk::RESOLUTION + 1; z++)
            {
                array<Chunk::DensityType, 8> values = c->voxelCubeAt(x, y, z);

                unsigned int caseIndex = c->caseIndexFromVoxel(values);

                if (caseIndex == 255)
                    continue; // solid block
                if (caseIndex == 0)
                    continue; // air block

                int numTriangles = case_to_numpolys[caseIndex];

                // for each triangle of the cube
                for (int t = 0; t < numTriangles; t++)
                {
                    Vector3I tri;

                    // for each edge of the cube a triangle vertex is on
                    for (int e = 0; e < 3; e++)
                    {
                        int edgeIndex = edge_connect_list[caseIndex][t][e];

                        Chunk::DensityType value1;
                        Chunk::DensityType value2;
                        Vector3I vec1;
                        Vector3I vec2;

                        switch (edgeIndex)
                        {
                            case 0:  value1 = values[0]; value2 = values[1]; vec1 = Vector3I(x, y, z); vec2 = Vector3I(x, y, z + 1); break;
                            case 1:  value1 = values[1]; value2 = values[2]; vec1 = Vector3I(x, y, z + 1); vec2 = Vector3I(x + 1, y, z + 1); break;
                            case 2:  value1 = values[2]; value2 = values[3]; vec1 = Vector3I(x + 1, y, z + 1); vec2 = Vector3I(x + 1, y, z); break;
                            case 3:  value1 = values[3]; value2 = values[0]; vec1 = Vector3I(x + 1, y, z); vec2 = Vector3I(x, y, z); break;

                            case 4:  value1 = values[4]; value2 = values[5]; vec1 = Vector3I(x, y + 1, z); vec2 = Vector3I(x, y + 1, z + 1); break;
                            case 5:  value1 = values[5]; value2 = values[6]; vec1 = Vector3I(x, y + 1, z + 1); vec2 = Vector3I(x + 1, y + 1, z + 1); break;
                            case 6:  value1 = values[6]; value2 = values[7]; vec1 = Vector3I(x + 1, y + 1, z + 1); vec2 = Vector3I(x + 1, y + 1, z); break;
                            case 7:  value1 = values[7]; value2 = values[4]; vec1 = Vector3I(x + 1, y + 1, z); vec2 = Vector3I(x, y + 1, z); break;

                            case 8:  value1 = values[0]; value2 = values[4]; vec1 = Vector3I(x, y, z); vec2 = Vector3I(x, y + 1, z); break;
                            case 9:  value1 = values[1]; value2 = values[5]; vec1 = Vector3I(x, y, z + 1); vec2 = Vector3I(x, y + 1, z + 1); break;
                            case 10: value1 = values[2]; value2 = values[6]; vec1 = Vector3I(x + 1, y, z + 1); vec2 = Vector3I(x + 1, y + 1, z + 1); break;
                            case 11: value1 = values[3]; value2 = values[7]; vec1 = Vector3I(x + 1, y, z); vec2 = Vector3I(x + 1, y + 1, z); break;

                            default: cerr << "Invalid edge index: " << edgeIndex << endl; break;
                        }

                        Vector3F vertex = interpolate(value1, value2, vec1, vec2);

                        // lookup this vertex
                        auto it = vertexMap.find(vertex);
                        if (it != vertexMap.end()) // we found it
                            tri[e] = it->second;
                        else
                        {
                            // calculate a new one
                            Vertex v;
                            v.position = c->toWorld(vertex);

                            Vector3F normal1 = getNormal(c, vec1);
                            Vector3F normal2 = getNormal(c, vec2);
                            v.normal = normalize(interpolate(value1, value2, normal1, normal2));

                            tri[e] = (unsigned int)c->vertices.size();
                            c->vertices.push_back(v);

                            vertexMap[v.position] = tri[e];
                        }
                    }

                    c->triangles.push_back(tri);
                }
            }
        }
    }

    cout << "Vertex map size " << vertexMap.size() << endl;
}

Vector3F ChunkCreator::getNormal(Chunk* c, const Vector3UI& v) const
{
    Vector3F grad;
    grad.x = c->voxelAt(v.x + 1, v.y, v.z) - c->voxelAt(v.x - 1, v.y, v.z);
    grad.y = c->voxelAt(v.x, v.y + 1, v.z) - c->voxelAt(v.x, v.y - 1, v.z);
    grad.z = c->voxelAt(v.x, v.y, v.z + 1) - c->voxelAt(v.x, v.y, v.z - 1);
    return normalize(grad);
}

