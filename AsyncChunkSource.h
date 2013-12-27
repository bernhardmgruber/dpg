#pragma once

#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "Chunk.h"

class AsyncChunkSource
{
public:
    AsyncChunkSource(unsigned int loaderThreads = 1);
    virtual ~AsyncChunkSource();

    Chunk* get(const Vector3I& chunkPos);

protected:
    virtual Chunk* getChunk(const Vector3I& chunkPos) = 0;

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
};

