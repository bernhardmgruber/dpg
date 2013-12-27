#pragma once

#include "AsyncChunkSource.h"


class ChunkCreator final : public AsyncChunkSource
{
public:
    ChunkCreator(unsigned int loaderThreads = 1);
    virtual ~ChunkCreator();

protected:
    virtual Chunk* getChunk(const Vector3I& chunkPos) override;

private:
    inline void marchChunk(Chunk* block);
    inline Vector3F getNormal(Chunk* c, const Vector3UI& v) const;

    template<typename T, typename FP>
    inline Vector3F interpolate(FP da, FP db, Vector3<T> va, Vector3<T> vb) const
    {
        FP part = fabs(da) / (fabs(da) + fabs(db));

        Vector3F result;
        result.x = va.x + (vb.x - va.x) * part;
        result.y = va.y + (vb.y - va.y) * part;
        result.z = va.z + (vb.z - va.z) * part;

        return result;
    }
};

