#pragma once

#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "Chunk.h"

class ChunkCreator final
{
public:
    ChunkCreator(unsigned int loaderThreads = 1);
    ~ChunkCreator();

    Chunk* get(const Vector3I& chunkPos);

private:

    /// loaded chunks, missing OpenGL initialization
    std::unordered_map<Vector3I, Chunk*> loadedChunks;
    std::unordered_set<Vector3I> enqueuedChunksSet;
    std::queue<Vector3I> enqueuedChunksQueue;

    /// A thread pool providing threads for loading
    std::vector<std::thread> loaderThreadPool;

    std::mutex loadedChunksMutex;
    std::condition_variable loadingChunkCV;

    bool shutdown;

    void loaderThreadMain();

    Chunk* createChunk(Vector3I chunkGridPosition);

    // chunk creation

    void marchChunk(Chunk* block);

    inline Vector3F getNormal(Chunk* c, const Vector3UI& v) const;

    template<typename T, typename FP>
    Vector3F interpolate(FP da, FP db, Vector3<T> va, Vector3<T> vb) const
    {
        FP part = fabs(da) / (fabs(da) + fabs(db));

        Vector3F result;
        result.x = va.x + (vb.x - va.x) * part;
        result.y = va.y + (vb.y - va.y) * part;
        result.z = va.z + (vb.z - va.z) * part;

        return result;
    }
};

