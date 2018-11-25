#pragma once

#include "mathlib.h"
#include "ChunkManager.h"

class Collider final
{
public:
    Collider(ChunkManager& mgr);
    ~Collider();

    glm::vec3 traceOnVoxels(glm::vec3 startPos, glm::vec3 endPos);

private:
    ChunkManager& mgr;
};

