#pragma once

#include <unordered_map>

#include "Chunk.h"
#include "ChunkCreator.h"
#include "ChunkSerializer.h"
#include "mathlib.h"

class ChunkManager final {
public:
	ChunkManager();
	ChunkManager(const ChunkManager& mgr) = delete;
	~ChunkManager();

	auto get(const glm::ivec3& pos) -> Chunk*;

private:
	ChunkMemoryFootprint getMemoryFootprint() const;

	ChunkCreator creator;
	ChunkSerializer serializer;

	std::unordered_map<glm::ivec3, Chunk> loadedChunks;
};