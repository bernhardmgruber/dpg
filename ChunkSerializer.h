#pragma once

#include <string>
#include <unordered_set>

#include "Chunk.h"

using namespace std;

class ChunkSerializer
{
public:
    ChunkSerializer(string chunkDir);
    ~ChunkSerializer();

    void storeChunk(const Chunk* chunk);
    Chunk* loadChunk(Chunk::IdType chunkId);

private:
    /**
    * All available chunks in the chunk directory
    */
    unordered_set<Chunk::IdType> availableChunks;
};

