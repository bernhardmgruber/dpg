#pragma once

#include "definitions.cl"
#include "intersection.cl"
#include "grid.cl"

typedef struct CellTraverser {
  int3 start;
  int3 directions; // step
  int3 stop;
  T3 next;
  T3 delta;
  bool reachedEnd;
} CellTraverser;

/**
* From: RegularGridCellTraverser::RegularGridCellTraverser()
*/
void cellTraverser_init(CellTraverser* const self, const Ray ray, const __global SubGrid* grid) {
  self->reachedEnd = false;

  T3 pos[2];
  if(intersectRayAABB(ray, grid->box, pos)) {
    self->start = grid_posToCellCoords(grid, pos[0]);

    // vectorization of loop
    T3 tmp = CONVERT_T3(grid->box.lower) + CONVERT_T3(self->start) * CONVERT_T(grid->cellSize);

    CMP_TYPE3 cmp = pos[0] < pos[1]; // 0 if the specified relation is false and a -1 (all bits set) if the specified relation is true
    self->stop = select((int3)(-1), convert_int3(grid->cellCount), convert_int3(cmp)); // Tn select(Tn a, Tn b, Tn c)  result[i] = if MSB of c[i] is set ? b[i] : a[i]
    self->directions = select((int3)(-1), (int3)(1), convert_int3(cmp));
    tmp += select((T3)(0.0f), (T3)grid->cellSize, cmp);

    //cmp = ray.direction != (float3)(0.0f);
    //self->next = select((float3)MAXFLOAT, (tmp - ray.origin) / ray.direction, cmp);
    self->next = ray.direction != (T3)(0.0f) ? (tmp - ray.origin) / ray.direction : (T3)MAX_T;
    self->delta = (CONVERT_T(grid->cellSize) * CONVERT_T3(self->directions)) / ray.direction;

    self->reachedEnd = self->start.x == self->stop.x || self->start.y == self->stop.y || self->start.z == self->stop.z;
  } else
    self->reachedEnd = true;
}

/**
* From: RegularGridCellTraverser::next()
*/
uint cellTraverser_next(CellTraverser* const self, const __global SubGrid* grid) {
  if(self->reachedEnd)
    return INVALID_INDEX;

  int axis = (self->next.x < self->next.y && self->next.x < self->next.z) ? 0 : (self->next.y < self->next.z) ? 1 : 2;

  uint index = grid_getCellIndex(grid, self->start.x, self->start.y, self->start.z);

  switch(axis) { // TODO: eliminate redudancy in this switch
  case 0:
    self->start.x += self->directions.x;
    if(self->start.x == self->stop.x)
      self->reachedEnd = true;
    else 
      self->next.x += self->delta.x;
    break;
  case 1:
    self->start.y += self->directions.y;
    if(self->start.y == self->stop.y)
      self->reachedEnd = true;
    else 
      self->next.y += self->delta.y;
    break;
  case 2:
    self->start.z += self->directions.z;
    if(self->start.z == self->stop.z)
      self->reachedEnd = true;
    else 
      self->next.z += self->delta.z;
    break;
  }

  return index;
}
