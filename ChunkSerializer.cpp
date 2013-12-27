#ifdef _WIN32
#include <windows.h>
#endif
#include <fstream>
#include "globals.h"

#include "ChunkSerializer.h"


ChunkSerializer::ChunkSerializer(string chunkDir)
: chunkDir(chunkDir)
{
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(chunkDir.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;
    else
    {
        do
        {
            static_assert(is_same<uint64_t, Chunk::IdType>::value, "Chunk::IdType is assumed to be uint64_t");
            char* p;
            Chunk::IdType id = strtoul(findData.cFileName, &p, 10);
            if (p != '\0')
            {
                cout << "Warning: " << findData.cFileName << " in chunk cache" << endl;
                continue;
            }

            availableChunks.insert(id);
        }
        while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }

#endif
}

ChunkSerializer::~ChunkSerializer()
{

}

bool ChunkSerializer::hasChunk(Chunk::IdType chunkId)
{
    return availableChunks.find(chunkId) != availableChunks.end();
}

void ChunkSerializer::storeChunk(const Chunk* chunk)
{
    if (!global::enableChunkCache)
        return;

    const unsigned int size = Chunk::RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging

    // write chunk to disk
    ofstream file(chunkDir + "/" + to_string(chunk->getId()), ios::binary);
    file.write((char*)chunk->densities, size * size * size * sizeof(Chunk::DensityType));
    file << (size_t)chunk->vertices.size();
    file.write((char*)chunk->vertices.data(), chunk->vertices.size() * sizeof(Vertex));
    file << (size_t)chunk->triangles.size();
    file.write((char*)chunk->triangles.data(), chunk->triangles.size() * sizeof(Vector3UI));
    file.close();
}

Chunk* ChunkSerializer::loadChunk(Chunk::IdType chunkId)
{
    if (!global::enableChunkCache || availableChunks.find(chunkId) == availableChunks.end())
        return nullptr; // this chunk is not available in the chunk directory

    // read chunk from disk
    Chunk* c = new Chunk(chunkId);
    const unsigned int size = Chunk::RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging

    ifstream file(chunkDir + "/" + to_string(chunkId), ios::binary);
    c->densities = new Chunk::DensityType[size * size * size];
    file.read((char*)c->densities, size * size * size * sizeof(Chunk::DensityType));
    size_t verticesCount;
    file >> verticesCount;
    c->vertices.resize(verticesCount);
    file.read((char*)c->vertices.data(), verticesCount * sizeof(Vertex));
    size_t trianglesCount;
    file >> trianglesCount;
    c->triangles.resize(trianglesCount);
    file.read((char*)c->triangles.data(), trianglesCount * sizeof(Vertex));
    file.close();

    return c;
}

//Chunk* ChunkSerializer::loadChunk(Vector3I& chunkGridPos)
//{
//    loadChunk(Chunk::ChunkGridCoordinateToId(chunkGridPos));
//}
