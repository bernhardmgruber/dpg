#pragma once

#include "AsyncChunkSource.h"

class ChunkCreator final : public AsyncChunkSource
{
public:
    ChunkCreator(unsigned int loaderThreads = 1);
    ~ChunkCreator();

protected:
    virtual Chunk* getChunk(const Vector3I& chunkPos) override;

private:
    inline void marchChunk(Chunk* block);
    inline Vector3F getNormal(Chunk* c, const Vector3UI& v) const;

    inline Vector3F interpolate(float da, float db, glm::vec3 va, glm::vec3 vb) const
    {
        float part = fabs(da) / (fabs(da) + fabs(db));

        Vector3F result;
        result.x = va.x + (vb.x - va.x) * part;
        result.y = va.y + (vb.y - va.y) * part;
        result.z = va.z + (vb.z - va.z) * part;

        return result;
    }
};

