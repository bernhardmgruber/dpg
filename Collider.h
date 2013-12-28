#pragma once

#include "mathlib.h"
#include "ChunkManager.h"

class Collider final
{
public:
    Collider(ChunkManager& mgr);
    ~Collider();

    Vector3F traceOnVoxels(Vector3F startPos, Vector3F endPos);

private:
    ChunkManager& mgr;
};

