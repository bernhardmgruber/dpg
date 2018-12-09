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


private:
	ChunkManager& mgr;
	Collider collider;
};
