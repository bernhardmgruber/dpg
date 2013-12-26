#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <condition_variable>
#include <thread>

#include "Chunk.h"
#include "ChunkCreator.h"
#include "ChunkSerializer.h"
#include "mathlib.h"

class ChunkManager
{
public:
    ChunkManager(unsigned int loaderThreads = 1);
    ~ChunkManager();

	Chunk* get(const Vector3I& pos);

private:
    ChunkCreator creator;
    ChunkSerializer serializer;

	/// fully loaded chunks
	std::unordered_map<Vector3I, Chunk*> chunks;

	/// loaded chunks, missing OpenGL initialization
	std::unordered_map<Vector3I, Chunk*> uninitializedChunks;

	std::unordered_set<Vector3I> enqueuedLoads;

	/// A thread pool providing threads for loading
	std::vector<std::thread> loaderThreadPool;

	std::mutex updatingChunks;

	std::mutex loadingChunkMutex;
	std::condition_variable loadingChunkCV;
	std::queue<Vector3I> enqueuedChunkLoads;

	bool shutdown;

	/**
	* Enqueues a chunk for loading.
	* This method executes asynchronously.
	*/
	void enqueueChunkLoad(const Vector3I& chunkPos);

	const ChunkMemoryFootprint getMemoryFootprint() const;

    void loaderThreadMain();
};