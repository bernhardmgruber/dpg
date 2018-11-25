#ifdef _WIN32
#include <windows.h>
#endif
#include <fstream>
#include <string>
#include "globals.h"
#include "utils.h"

#include "ChunkSerializer.h"


ChunkSerializer::ChunkSerializer(string chunkDir)
: chunkDir(chunkDir)
{
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((chunkDir + "*").c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;
    else
    {
        do
        {
            if (!strcmp(findData.cFileName, ".") || !strcmp(findData.cFileName, ".."))
                continue;

            static_assert(is_same<uint64_t, Chunk::IdType>::value, "Chunk::IdType is assumed to be uint64_t");
            char* p;
            Chunk::IdType id = strtoull(findData.cFileName, &p, 16);
            if (*p != '\0')
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
{}

bool ChunkSerializer::hasChunk(const glm::ivec3& chunkPos)
{
    return availableChunks.find(Chunk::ChunkGridCoordinateToId(chunkPos)) != availableChunks.end();
}

void ChunkSerializer::storeChunk(const Chunk* chunk)
{
    if (!global::enableChunkCache)
        return;

    const unsigned int size = Chunk::RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging

    // write chunk to disk
    ofstream file(chunkDir + toHexString(chunk->getId()), ios::binary);
    file.write((char*)chunk->densities, size * size * size * sizeof(Chunk::DensityType));
    file << (size_t)chunk->vertices.size();
    file.write((char*)chunk->vertices.data(), chunk->vertices.size() * sizeof(Vertex));
    file << (size_t)chunk->triangles.size();
    file.write((char*)chunk->triangles.data(), chunk->triangles.size() * sizeof(glm::uvec3));
    file.close();

    cout << "Wrote chunk from disk: " << chunk->getChunkGridPositon() << endl;
}

Chunk* ChunkSerializer::getChunk(const glm::ivec3& chunkPos)
{
    Chunk::IdType chunkId = Chunk::ChunkGridCoordinateToId(chunkPos);

    if (!global::enableChunkCache || availableChunks.find(chunkId) == availableChunks.end())
        return nullptr; // this chunk is not available in the chunk directory

    // read chunk from disk
    Chunk* c = new Chunk(chunkId);
    const unsigned int size = Chunk::RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging

    ifstream file(chunkDir + toHexString(chunkId), ios::binary);
    if (!file)
        throw runtime_error("could not open chunk file " + chunkDir + to_string(chunkId));
    c->densities = new Chunk::DensityType[size * size * size];
    file.read((char*)c->densities, size * size * size * sizeof(Chunk::DensityType));
    size_t verticesCount = 0;
    file >> verticesCount;
    if (verticesCount > 0)
    {
        c->vertices.resize(verticesCount);
        file.read((char*)c->vertices.data(), verticesCount * sizeof(Vertex));
    }
    size_t trianglesCount = 0;
    file >> trianglesCount;
    if (trianglesCount > 0)
    {
        c->triangles.resize(trianglesCount);
        file.read((char*)c->triangles.data(), trianglesCount * sizeof(Vertex));
    }
    file.close();

    cout << "Read chunk from disk:  " << chunkPos << endl;

    return c;
}
