#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include <tuple>

#include "ChunkLoader.h"
#include "Chunk.h"

class World
{
    public:
        World();
        virtual ~World();

        void update();
        void render();

		const ChunkMemoryFootprint getMemoryFootprint() const;

        //
        // movement, collision detection, etc.
        //

        /**
        * Checks whether or not a position is inside the solid world (or surface) or not (air).
        */
        bool isAir(const Vector3F& pos) const;

        /**
        * Moves the position with the bounding box to the nearest non solid position.
        */
        Vector3F getNearestNonSolidPos(const Vector3F& pos, BoundingBox& box) const;

        /**
        * Performs the move from src to dst with the given bounding box inside the world.
        * 
        * @return Returns the final destination position after possible collisions, sliding, etc.
        */
        Vector3F move(const Vector3F src, const BoundingBox& box, const Vector3F dst) const;

    private:
		mutable ChunkLoader loader;

        /** Holds all chunks that need to be rendered. This list is generated during Update() and used by Render(). */
        std::vector<Chunk*> renderList;

		Vector3I lastCameraChunk;
		bool renderListComplete;

        void buildRenderList(const Vector3I& cameraChunkPos);

        Vector3I getChunkPos(const Vector3F& pos) const;
};
