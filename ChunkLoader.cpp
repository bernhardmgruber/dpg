#include <thread>
#include <future>

#include "ChunkLoader.h"

using namespace std;

ChunkLoader::ChunkLoader(unsigned int loaderThreads)
	: shutdown(false)
{
	// infulenced by: http://progsch.net/wordpress/?p=81
	auto func = [this]()
	{
		while(true)
		{
			Vector3I chunkPos;

			// wait for
			{
				unique_lock<mutex> lock(loadingChunkMutex);

				// check for shutdown (shutdown might have been set during unlocked chunk loading)
				if(shutdown)
				{
					cout << "Loader thread #" << std::this_thread::get_id() << " exited 2" << endl;
					return;
				}

				cout << "Loader thread #" << std::this_thread::get_id() << " is ready" << endl;
				loadingChunkCV.wait(lock, [this](){ return enqueuedChunkLoads.size() > 0 || shutdown; });
				cout << "Loader thread #" << std::this_thread::get_id() << " notified" << endl;

				// check for shutdown
				if(shutdown)
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
			uninitializedChunks[c->getPosition()] = c;
			updatingChunks.unlock();
			cout << "Loader thread #" << std::this_thread::get_id() << " finished loading chunk " << chunkPos << endl;
		}
	};

	// create pool threads
	for(unsigned int i = 0; i < loaderThreads; i++)
		loaderThreadPool.push_back(thread(func));
}

ChunkLoader::~ChunkLoader()
{
	// join
	{
		unique_lock<mutex> lock(loadingChunkMutex);

		shutdown = true;
		loadingChunkCV.notify_all();
	}

	for(thread& t : loaderThreadPool)
		t.join();

	for(auto& pair : chunks)
		delete pair.second;
	for(auto& pair : uninitializedChunks)
		delete pair.second;
}

Chunk* ChunkLoader::get(const Vector3I& pos)
{
	// try to find in fully loaded chunks
	auto it = chunks.find(pos);
	if(it != chunks.end())
	{
		// this chunk is loaded
		return it->second;
	}

	// try to find in uninitialized chunks
	Chunk* c = nullptr;

	updatingChunks.lock();
	it = uninitializedChunks.find(pos);
	if(it != uninitializedChunks.end())
	{
		c = it->second;
		uninitializedChunks.erase(it);
	}
	updatingChunks.unlock();

	if(c)
	{
		c->createBuffers();
		chunks[c->getPosition()] = c;
		return c;
	}

	// this chunk has not been loaded
	enqueueChunkLoad(pos);
	return nullptr;
}

void ChunkLoader::enqueueChunkLoad(const Vector3I& chunkPos)
{
	loadingChunkMutex.lock();
	enqueuedChunkLoads.push(chunkPos);
	loadingChunkMutex.unlock();
	loadingChunkCV.notify_one();
}

const ChunkMemoryFootprint ChunkLoader::getMemoryFootprint() const
{
	ChunkMemoryFootprint mem = {0};

	for(const auto& pair : chunks)
	{
		const ChunkMemoryFootprint& cmem = pair.second->getMemoryFootprint();
		mem.densityValues += cmem.densityValues;
		mem.densityValueSize = cmem.densityValueSize;
		mem.triangles += cmem.triangles;
		mem.triangleSize = cmem.triangleSize;
	}

	return mem;
}