#pragma once

#include "ChunkManager.h"
#include "Collider.h"

/**
* The main physics class
*/
class WorldPhysics final
{
public:
	WorldPhysics(ChunkManager& mgr);
	~WorldPhysics();

	Chunk::VoxelType categorizeWorldPosition(const Vector3F& pos) const;

	/**
	* Moves the position with the bounding box to the nearest non solid position.
	*/
	Vector3F getNearestNonSolidPos(const Vector3F& pos, BoundingBox& box) const;

	/**
	* Performs the move from src to dst inside the world.
	*
	* @return Returns the final destination position after possible collisions, sliding, etc.
	*/
	Vector3F move(const Vector3F src, const Vector3F dst) const;

	/**
	* Performs the move from src to dst with the given bounding box inside the world.
	*
	* @return Returns the final destination position after possible collisions, sliding, etc.
	*/
	Vector3F move(const Vector3F src, const BoundingBox& box, const Vector3F dst) const;

	Vector3I getChunkPos(const Vector3F& pos) const;

private:
	ChunkManager& mgr;
	Collider collider;
};

