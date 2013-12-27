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

class ChunkManager final
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

	const ChunkMemoryFootprint getMemoryFootprint() const;
};