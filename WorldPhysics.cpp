#include "utils.h"

#include "WorldPhysics.h"


WorldPhysics::WorldPhysics(ChunkManager& mgr)
:mgr(mgr), collider(mgr)
{}


WorldPhysics::~WorldPhysics()
{}

Chunk::VoxelType WorldPhysics::categorizeWorldPosition(const Vector3F& pos) const
{
    const Vector3I chunkPos = getChunkPos(pos);
    const Chunk* chunk = mgr.get(chunkPos);

    return chunk->categorizeWorldPosition(pos);
}

Vector3F WorldPhysics::getNearestNonSolidPos(const Vector3F& pos, BoundingBox& box) const
{
    return pos;
}

Vector3F WorldPhysics::move(const Vector3F src, const Vector3F dst) const
{
    return dst;
}

Vector3F WorldPhysics::move(const Vector3F src, const BoundingBox& box, const Vector3F dst) const
{
    return dst;
}

Vector3I WorldPhysics::getChunkPos(const Vector3F& pos) const
{
    Vector3F chunkPos = pos / Chunk::SIZE;
    return Vector3I(roundToInt(chunkPos.x), roundToInt(chunkPos.y), roundToInt(chunkPos.z));
}
