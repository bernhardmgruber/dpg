#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "Chunk.h"

class AsyncChunkSource {
public:
	AsyncChunkSource(unsigned int loaderThreads = 1);
	virtual ~AsyncChunkSource();

	Chunk* get(const glm::ivec3& chunkPos);

protected:
	virtual Chunk* getChunk(const glm::ivec3& chunkPos) = 0;

private:
	/// loaded chunks, missing OpenGL initialization
	std::unordered_map<glm::ivec3, Chunk*> loadedChunks;
	std::unordered_set<glm::ivec3> enqueuedChunksSet;
	std::queue<glm::ivec3> enqueuedChunksQueue;

	/// A thread pool providing threads for loading
	std::vector<std::thread> loaderThreadPool;

	std::mutex loadedChunksMutex;
	std::condition_variable loadingChunkCV;

	bool shutdown = false;

	void loaderThreadMain();
};
