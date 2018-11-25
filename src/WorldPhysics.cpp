#include "utils.h"

#include "WorldPhysics.h"


WorldPhysics::WorldPhysics(ChunkManager& mgr)
	: mgr(mgr), collider(mgr) {}


WorldPhysics::~WorldPhysics() {}

Chunk::VoxelType WorldPhysics::categorizeWorldPosition(const glm::vec3& pos) const {
	const glm::ivec3 chunkPos = getChunkPos(pos);
	const Chunk* chunk = mgr.get(chunkPos);

	return chunk->categorizeWorldPosition(pos);
}

glm::vec3 WorldPhysics::getNearestNonSolidPos(const glm::vec3& pos, BoundingBox& box) const {
	return pos;
}

glm::vec3 WorldPhysics::move(const glm::vec3 src, const glm::vec3 dst) const {
	return dst;
}

glm::vec3 WorldPhysics::move(const glm::vec3 src, const BoundingBox& box, const glm::vec3 dst) const {
	return dst;
}

glm::ivec3 WorldPhysics::getChunkPos(const glm::vec3& pos) const {
	glm::vec3 chunkPos = pos / Chunk::SIZE;
	return glm::ivec3(roundToInt(chunkPos.x), roundToInt(chunkPos.y), roundToInt(chunkPos.z));
}
