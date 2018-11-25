#pragma once

#include "AsyncChunkSource.h"

class ChunkCreator final : public AsyncChunkSource
{
public:
    ChunkCreator(unsigned int loaderThreads = 1);
    ~ChunkCreator();

protected:
    virtual Chunk* getChunk(const glm::ivec3& chunkPos) override;

private:
    inline void marchChunk(Chunk* block);
    inline glm::vec3 getNormal(Chunk* c, const glm::uvec3& v) const;

    inline glm::vec3 interpolate(float da, float db, glm::vec3 va, glm::vec3 vb) const
    {
        float part = fabs(da) / (fabs(da) + fabs(db));

        glm::vec3 result;
        result.x = va.x + (vb.x - va.x) * part;
        result.y = va.y + (vb.y - va.y) * part;
        result.z = va.z + (vb.z - va.z) * part;

        return result;
    }
};

