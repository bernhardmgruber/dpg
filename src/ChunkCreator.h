#pragma once

#include "AsyncChunkSource.h"

class ChunkCreator final : public AsyncChunkSource {
protected:
	virtual auto getChunk(const glm::ivec3& chunkPos) -> Chunk override;
};
