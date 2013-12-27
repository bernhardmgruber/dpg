#pragma once

#include "AsyncChunkSource.h"

using namespace std;

class ChunkSerializer final : public AsyncChunkSource
{
public:
    ChunkSerializer(string chunkDir);
    virtual ~ChunkSerializer();

    bool hasChunk(const Vector3I& chunkPos);
    void storeChunk(const Chunk* chunk);

protected:
    virtual Chunk* getChunk(const Vector3I& chunkPos) override;

private:
    std::string chunkDir;

    /**
    * All available chunks in the chunk directory
    */
    unordered_set<Chunk::IdType> availableChunks;
};

