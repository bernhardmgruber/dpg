#pragma once

#include "ChunkManager.h"
#include "mathlib.h"

class Collider final {
public:
	Collider(ChunkManager& mgr);
	~Collider();

	glm::vec3 traceOnVoxels(glm::vec3 startPos, glm::vec3 endPos);

private:
	ChunkManager& mgr;
};
