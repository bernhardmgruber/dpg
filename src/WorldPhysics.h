#pragma once

#include "ChunkManager.h"
#include "Collider.h"

/**
* The main physics class
*/
class WorldPhysics final {
public:
	WorldPhysics(ChunkManager& mgr);
	~WorldPhysics();

	Chunk::VoxelType categorizeWorldPosition(const glm::vec3& pos) const;

	/**
	* Moves the position with the bounding box to the nearest non solid position.
	*/
	glm::vec3 getNearestNonSolidPos(const glm::vec3& pos, BoundingBox& box) const;

	/**
	* Performs the move from src to dst inside the world.
	*
	* @return Returns the final destination position after possible collisions, sliding, etc.
	*/
	glm::vec3 move(const glm::vec3 src, const glm::vec3 dst) const;

	/**
	* Performs the move from src to dst with the given bounding box inside the world.
	*
	* @return Returns the final destination position after possible collisions, sliding, etc.
	*/
	glm::vec3 move(const glm::vec3 src, const BoundingBox& box, const glm::vec3 dst) const;

	glm::ivec3 getChunkPos(const glm::vec3& pos) const;

private:
	ChunkManager& mgr;
	Collider collider;
};
