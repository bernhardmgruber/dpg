#include "utils.h"

#include "WorldPhysics.h"


WorldPhysics::WorldPhysics(ChunkManager& mgr)
	: mgr(mgr), collider(mgr) {}


WorldPhysics::~WorldPhysics() = default;
