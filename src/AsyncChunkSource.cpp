#include "AsyncChunkSource.h"

auto AsyncChunkSource::get(const glm::ivec3& chunkPos) -> std::optional<Chunk> {
	// try to find in loaded chunks
	const auto it = loadedChunks.find(chunkPos);
	if (it != loadedChunks.end()) {
		auto& f = it->second;
		if (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			Chunk c = f.get();
			loadedChunks.erase(it);

			// init chunk before returning it
			c.createBuffers();
			return c;
		}
	} else {
		loadedChunks[chunkPos] = std::async(std::launch::async, [=] {
			return getChunk(chunkPos);
		});
	}

	return {};
}

void AsyncChunkSource::clear() {
	loadedChunks.clear();
}
