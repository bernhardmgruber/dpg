#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <condition_variable>
#include <thread>

#include "Chunk.h"
#include "mathlib.h"

struct Vector3IHash
{
	size_t operator() (const Vector3I& v) const { return v.x ^ (v.y << 11) ^ (v.z << 22); }
};

class ChunkLoader
{
public:
	ChunkLoader(unsigned int loaderThreads = 1);
	~ChunkLoader();

	Chunk* get(const Vector3I& pos);

private:
	/// fully loaded chunks
	std::unordered_map<Vector3I, Chunk*, Vector3IHash> chunks;

	/// loaded chunks, missing OpenGL initialization
	std::unordered_map<Vector3I, Chunk*, Vector3IHash> uninitializedChunks;

	std::unordered_set<Vector3I, Vector3IHash> enqueuedLoads;

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
};