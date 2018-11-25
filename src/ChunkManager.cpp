#include "globals.h"
#include <future>
#include <thread>

#include "ChunkManager.h"

ChunkManager::ChunkManager()
	: serializer("chunks") {
}

ChunkManager::~ChunkManager() {
	for (const auto& [i, chunk] : loadedChunks)
		serializer.storeChunk(chunk);
}

Chunk* ChunkManager::get(const glm::ivec3& pos) {
	// check if chunk is available
	if (const auto it = loadedChunks.find(pos); it != loadedChunks.end())
		return &it->second;

	// chunk was not found, ask the cache on disk
	if (global::enableChunkCache && serializer.hasChunk(pos)) {
		if (auto c = serializer.get(pos))
			return &(loadedChunks[pos] = std::move(*c));
		else
			return nullptr;
	}

	// not found on disk, create it
	if (auto c = creator.get(pos))
		return &(loadedChunks[pos] = std::move(*c));
	else
		return nullptr;
}

ChunkMemoryFootprint ChunkManager::getMemoryFootprint() const {
	ChunkMemoryFootprint mem{};

	for (const auto& [i, chunk] : loadedChunks) {
		const auto& cmem = chunk.getMemoryFootprint();
		mem.densityValues += cmem.densityValues;
		mem.densityValueSize = cmem.densityValueSize;
		mem.triangles += cmem.triangles;
		mem.triangleSize = cmem.triangleSize;
	}

	return mem;
}
