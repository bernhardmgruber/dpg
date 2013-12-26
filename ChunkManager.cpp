#include <thread>
#include <future>

#include "ChunkManager.h"

using namespace std;

ChunkManager::ChunkManager(unsigned int loaderThreads)
: shutdown(false), serializer("chunks")
{
    // create pool threads
    for (unsigned int i = 0; i < loaderThreads; i++)
        loaderThreadPool.push_back(thread(&ChunkManager::loaderThreadMain, this));
}

ChunkManager::~ChunkManager()
{
    // join
    {
        unique_lock<mutex> lock(loadingChunkMutex);
        shutdown = true;
    }
    loadingChunkCV.notify_all();

    for (thread& t : loaderThreadPool)
        t.join(); // for some reason, this blocks forever =/

    for (auto& pair : chunks)
        delete pair.second;
    for (auto& pair : uninitializedChunks)
        delete pair.second;
}

Chunk* ChunkManager::get(const Vector3I& pos)
{
    // try to find in fully loaded chunks
    auto it = chunks.find(pos);
    if (it != chunks.end())
    {
        // this chunk is loaded
        return it->second;
    }

    // try to find in uninitialized chunks
    Chunk* c = nullptr;

    {
        lock_guard<mutex> lock(updatingChunks);

        it = uninitializedChunks.find(pos);
        if (it != uninitializedChunks.end())
        {
            c = it->second;
            uninitializedChunks.erase(it);
        }
    }

    if (c)
    {
        c->createBuffers();
        chunks[c->getChunkGridPositon()] = c;
        return c;
    }

    // this chunk has not been loaded
    enqueueChunkLoad(pos);
    return nullptr;
}

void ChunkManager::enqueueChunkLoad(const Vector3I& chunkPos)
{
    if (enqueuedLoads.find(chunkPos) != enqueuedLoads.end())
        return; // chunk is already enqueued for loading
    enqueuedLoads.insert(chunkPos);

    {
        lock_guard<mutex> lock(loadingChunkMutex);
        enqueuedChunkLoads.push(chunkPos);
    }
    loadingChunkCV.notify_one();
}

const ChunkMemoryFootprint ChunkManager::getMemoryFootprint() const
{
    ChunkMemoryFootprint mem = { 0 };

    for (const auto& pair : chunks)
    {
        const ChunkMemoryFootprint& cmem = pair.second->getMemoryFootprint();
        mem.densityValues += cmem.densityValues;
        mem.densityValueSize = cmem.densityValueSize;
        mem.triangles += cmem.triangles;
        mem.triangleSize = cmem.triangleSize;
    }

    return mem;
}

void ChunkManager::loaderThreadMain()
{
    // influenced by: http://progsch.net/wordpress/?p=81
    while (true)
    {
        Vector3I chunkPos;

        // wait for
        {
            unique_lock<mutex> lock(loadingChunkMutex);

            // check for shutdown (shutdown might have been set during unlocked chunk loading)
            if (shutdown)
            {
                cout << "Loader thread #" << std::this_thread::get_id() << " exited 2" << endl;
                return;
            }

            cout << "Loader thread #" << std::this_thread::get_id() << " is ready" << endl;
            while (enqueuedChunkLoads.size() == 0 && !shutdown)
            {
                loadingChunkCV.wait(lock);
                cout << "Loader thread #" << std::this_thread::get_id() << " notified" << endl;
            }

            // check for shutdown
            if (shutdown)
            {
                cout << "Loader thread #" << std::this_thread::get_id() << " exited 1" << endl;
                return;
            }

            // dequeue load request
            chunkPos = enqueuedChunkLoads.front();
            enqueuedChunkLoads.pop();
            cout << "Loader thread #" << std::this_thread::get_id() << " starts loading chunk " << chunkPos << endl;
        }

        // do load
        Chunk* c = new Chunk(chunkPos);
        updatingChunks.lock();
        uninitializedChunks[c->getChunkGridPositon()] = c;
        updatingChunks.unlock();
        cout << "Loader thread #" << std::this_thread::get_id() << " finished loading chunk " << chunkPos << endl;
    }
}