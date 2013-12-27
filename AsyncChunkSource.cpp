#include "AsyncChunkSource.h"

using namespace std;

AsyncChunkSource::AsyncChunkSource(unsigned int loaderThreads)
{
    // create pool threads
    for (unsigned int i = 0; i < loaderThreads; i++)
        loaderThreadPool.push_back(thread(&AsyncChunkSource::loaderThreadMain, this));
}

AsyncChunkSource::~AsyncChunkSource()
{
    // join
    {
        unique_lock<mutex> lock(loadedChunksMutex);
        shutdown = true;
        loadingChunkCV.notify_all();
    }

    for (thread& t : loaderThreadPool)
        t.join();

    for (auto& pair : loadedChunks)
        delete pair.second;
}

Chunk* AsyncChunkSource::get(const Vector3I& chunkPos)
{
    {
        lock_guard<mutex> lock(loadedChunksMutex);

        // try to find in loaded chunks
        auto it = loadedChunks.find(chunkPos);
        if (it != loadedChunks.end())
        {
            Chunk* c = it->second;
            loadedChunks.erase(it);

            // init chunk before returning it
            c->createBuffers();
            return c;
        }

        // add to chunks which should be loaded if it has not already been added
        if (enqueuedChunksSet.find(chunkPos) == enqueuedChunksSet.end())
        {
            enqueuedChunksQueue.push(chunkPos);
            enqueuedChunksSet.insert(chunkPos);
            loadingChunkCV.notify_one();
        }

        return nullptr;
    }
}

void AsyncChunkSource::loaderThreadMain()
{
    // influenced by: http://progsch.net/wordpress/?p=81
    while (true)
    {
        Vector3I chunkPos;

        // wait for
        {
            unique_lock<mutex> lock(loadedChunksMutex);

            while (enqueuedChunksSet.size() == 0 && !shutdown)
            {
                //cout << "Loader thread #" << std::this_thread::get_id() << " is ready" << endl;
                loadingChunkCV.wait(lock);
                //cout << "Loader thread #" << std::this_thread::get_id() << " notified" << endl;
            }

            // check for shutdown
            if (shutdown)
            {
                //cout << "Loader thread #" << std::this_thread::get_id() << " exited" << endl;
                return;
            }

            // dequeue load request
            chunkPos = enqueuedChunksQueue.front();
            enqueuedChunksQueue.pop();
            //cout << "Loader thread #" << std::this_thread::get_id() << " starts loading chunk " << chunkPos << endl;
        }

        // do load
        Chunk* c = getChunk(chunkPos);

        {
            lock_guard<mutex> lock(loadedChunksMutex);
            loadedChunks[c->getChunkGridPositon()] = c;
            enqueuedChunksSet.erase(enqueuedChunksSet.find(chunkPos));
            //cout << "Loader thread #" << std::this_thread::get_id() << " finished loading chunk " << chunkPos << endl;
        }
    }
}
