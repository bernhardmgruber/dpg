#ifndef MARCHINGCUBES_H
#define MARCHINGCUBES_H

#define CHUNK_SIZE 33

#include <vector>

#include "mathlib.h"

std::vector<Triangle> MarchBlock(float* cube, int size);

#endif
