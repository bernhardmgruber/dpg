#pragma once

#include "Chunk.h"

class ChunkCreator final
{
public:
    Chunk* createChunk(Vector3I chunkGridPosition);

private:
    void marchChunk(Chunk* block);

    inline Vector3F getNormal(Chunk* c, const Vector3UI& v) const;

    template<typename T, typename FP>
    Vector3F interpolate(FP da, FP db, Vector3<T> va, Vector3<T> vb) const
    {
        FP part = fabs(da) / (fabs(da) + fabs(db));
        return (vb - va) * part;
    }
};

