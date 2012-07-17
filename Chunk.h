#ifndef CHUNK_H
#define CHUNK_H

#include <vector>

#include "mathlib.h"

class Chunk
{
    public:
        static Chunk* fromNoise(Vector3D center);

        /**
         * Converts a density value coordinate to a world coordinate.
         */
        Vector3D ToWorld(int x, int y, int z);

        static const float SIZE;
        static const int RESOLUTION;

        Vector3D GetCenter();

        void Render();

    private:
        Chunk();
        Vector3D center;

        std::vector<Triangle> triangles;

    friend void MarchChunk(Chunk& c, float* block);
};

#endif // CHUNK_H
