#ifndef MARCHINGCUBES_H
#define MARCHINGCUBES_H

#define CHUNK_SIZE 33

#include <vector>

struct Vertex
{
    float x, y, z;

    Vertex() {};

    Vertex(float x, float y, float z)
        : x(x), y(y), z(z)
    {};
};

struct Triangle
{
    Vertex vertices[3];
};

std::vector<Triangle> MarchBlock(float block[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]);

#endif
