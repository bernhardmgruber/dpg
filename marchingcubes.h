#ifndef MARCHINGCUBES_H
#define MARCHINGCUBES_H

#define CHUNK_SIZE 33

#include <vector>

#include "mathlib.h"
#include "Chunk.h"

void MarchChunk(Chunk& c, float* cube);

#endif
