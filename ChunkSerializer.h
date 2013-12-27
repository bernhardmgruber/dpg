#pragma once

#include <string>
#include <unordered_set>

#include "Chunk.h"

using namespace std;

class ChunkSerializer final
{
public:
    ChunkSerializer(string chunkDir);
    ~ChunkSerializer();

    bool hasChunk(Chunk::IdType chunkId);

    void storeChunk(const Chunk* chunk);
    Chunk* loadChunk(Chunk::IdType chunkId);
    //Chunk* loadChunk(Vector3I& chunkGridPos);

private:
    std::string chunkDir;

    /**
    * All available chunks in the chunk directory
    */
    unordered_set<Chunk::IdType> availableChunks;
};

