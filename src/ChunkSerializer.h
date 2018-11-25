#pragma once

#include "AsyncChunkSource.h"

using namespace std;

class ChunkSerializer final : public AsyncChunkSource {
public:
	ChunkSerializer(string chunkDir);
	virtual ~ChunkSerializer();

	bool hasChunk(const glm::ivec3& chunkPos);
	void storeChunk(const Chunk* chunk);

protected:
	virtual Chunk* getChunk(const glm::ivec3& chunkPos) override;

private:
	std::string chunkDir;

	/**
    * All available chunks in the chunk directory
    */
	unordered_set<Chunk::IdType> availableChunks;
};
