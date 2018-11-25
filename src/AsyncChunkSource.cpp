#include "AsyncChunkSource.h"

auto AsyncChunkSource::get(const glm::ivec3& chunkPos) -> std::optional<Chunk> {
	// try to find in loaded chunks
	const auto it = loadedChunks.find(chunkPos);
	if (it != loadedChunks.end() && it->second._Is_ready()) {
		Chunk c = it->second.get();
		loadedChunks.erase(it);

		// init chunk before returning it
		c.createBuffers();
		return c;
	}

	loadedChunks[chunkPos] = std::async(std::launch::async, [=] {
		return getChunk(chunkPos);
	});

	return {};
}
