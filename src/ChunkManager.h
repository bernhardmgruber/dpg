#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Chunk.h"
#include "ChunkCreator.h"
#include "ChunkSerializer.h"
#include "mathlib.h"

class ChunkManager final {
public:
	ChunkManager();
	ChunkManager(const ChunkManager& mgr) = delete;
	~ChunkManager();

	Chunk* get(const glm::ivec3& pos);

private:
	ChunkCreator creator;
	ChunkSerializer serializer;

	/// fully loaded chunks
	std::unordered_map<glm::ivec3, Chunk*> chunks;

	const ChunkMemoryFootprint getMemoryFootprint() const;
};