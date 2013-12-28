#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include <tuple>

#include "ChunkManager.h"
#include "WorldPhysics.h"

class World
{
    public:
        World();
        virtual ~World();

        void update();
        void render();

		const ChunkMemoryFootprint getMemoryFootprint() const;

    private:
		mutable ChunkManager loader;
        mutable WorldPhysics physics;


        /** Holds all chunks that need to be rendered. This list is generated during Update() and used by Render(). */
        std::vector<Chunk*> renderList;

		Vector3I lastCameraChunk;
		bool renderListComplete;

        void buildRenderList(const Vector3I& cameraChunkPos);
};
