#pragma once

#include <future>
#include <optional>
#include <unordered_map>

#include "Chunk.h"

class AsyncChunkSource {
public:
	AsyncChunkSource() = default;
	virtual ~AsyncChunkSource() = default;

	auto get(const glm::ivec3& chunkPos) -> std::optional<Chunk>;

protected:
	virtual auto getChunk(const glm::ivec3& chunkPos) -> Chunk = 0;

private:
	std::unordered_map<glm::ivec3, std::future<Chunk>> loadedChunks;
};
