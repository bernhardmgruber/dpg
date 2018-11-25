#pragma once

#include <filesystem>
#include <unordered_set>

#include "AsyncChunkSource.h"

using namespace std;

class ChunkSerializer final : public AsyncChunkSource {
public:
	ChunkSerializer(std::filesystem::path chunkDir);
	virtual ~ChunkSerializer();

	bool hasChunk(const glm::ivec3& chunkPos);
	void storeChunk(const Chunk& chunk);

protected:
	virtual auto getChunk(const glm::ivec3& chunkPos) -> Chunk override;

private:
	std::filesystem::path m_chunkDir;

	/**
	* All available chunks in the chunk directory
	*/
	std::unordered_set<Chunk::IdType> availableChunks;
};
