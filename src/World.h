#pragma once

#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ChunkManager.h"
#include "WorldPhysics.h"

class World {
public:
	World();

	void update();
	void render();

	ChunkMemoryFootprint getMemoryFootprint() const;

private:
	mutable ChunkManager chunks;
	mutable WorldPhysics physics;


	/** Holds all chunks that need to be rendered. This list is generated during Update() and used by Render(). */
	std::vector<Chunk*> renderList;

	glm::ivec3 lastCameraChunk{};
	bool renderListComplete;

	void buildRenderList(const glm::ivec3& cameraChunkPos);
};
