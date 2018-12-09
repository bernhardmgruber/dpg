#pragma once

#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ChunkManager.h"

class Camera;

class World {
public:
	World();

	void update(Camera& camera);
	void render();
	void renderAuxiliary();

	void clearChunks();

	auto trace(glm::vec3 start, glm::vec3 end) const -> glm::vec3;

	auto categorizeWorldPosition(const glm::vec3& pos) const -> Chunk::VoxelType;

	// Moves the position with the bounding box to the nearest non solid position.
	glm::vec3 getNearestNonSolidPos(const glm::vec3& pos, BoundingBox& box) const;

	glm::ivec3 getChunkPos(const glm::vec3& pos) const;
	glm::ivec3 getVoxelPos(const glm::vec3& pos) const;

private:
	ChunkManager chunks;

	/** Holds all chunks that need to be rendered. This list is generated during Update() and used by Render(). */
	std::vector<Chunk*> renderList;

	glm::ivec3 lastCameraChunk{};
	bool renderListComplete = false;

	void buildRenderList(const glm::ivec3& cameraChunkPos);
};
