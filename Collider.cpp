#include "Collider.h"

Collider::Collider(ChunkManager& mgr)
: mgr(mgr)
{}

Collider::~Collider()
{}

Vector3F Collider::traceOnVoxels(Vector3F startPos, Vector3F endPos)
{
	return endPos;
}
