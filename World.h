#pragma once

#include <vector>
#include <unordered_map>
#include <tuple>

#include "Chunk.h"

struct Vector3IHash
{
    size_t operator() (const Vector3I& v) const { return v.x ^ v.y ^ v.z; }
};

struct Vector3IEqual
{
    bool operator() (const Vector3I& v1, const Vector3I& v2) const { return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z; }
};

class World
{
    public:
        World();
        virtual ~World();

        void update();
        void render();

    private:

        void recursiveChunkCheck(const Vector3I& chunkPos, int level);

        /** Holds all loaded chunks */
		std::unordered_map<Vector3I, std::tuple<Chunk*, bool>, Vector3IHash, Vector3IEqual> chunks;

        /** Holds all chunks that need to be rendered. This list is generated during Update() and used by Render(). */
        std::vector<Chunk*> renderList;
};
