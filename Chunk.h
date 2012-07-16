#ifndef CHUNK_H
#define CHUNK_H

#include <vector>

#include "mathlib.h"

class Chunk
{
    public:
        static Chunk* fromNoise(Vector3D center);

        static const float SIZE;
        static const int RESOLUTION;

        void Render();

    private:
        Chunk();
        Vector3D center;

        std::vector<Triangle> triangles;
};

#endif // CHUNK_H
