#ifndef CHUNK_H
#define CHUNK_H

#include <vector>

#include "mathlib.h"

class Chunk
{
    public:
        static Chunk* fromNoise(Vector3I center);

        /**
         * Converts a density value coordinate to a world coordinate.
         */
        Vector3F ToWorld(int x, int y, int z);
        Vector3F ToWorld(Vector3F in);

        static const float SIZE;
        static const int RESOLUTION;

        Vector3I GetCenter();

        void Render();

    private:
        Chunk();

        Vector3I center;

        std::vector<Triangle> triangles;

        /** Set to true by World::RecursiceChunkCheck to avoid infinite recursion. */
        bool marked;

    friend void MarchChunk(Chunk& c, float* block);
    friend class World;
};

#endif // CHUNK_H
