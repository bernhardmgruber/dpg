#include <iostream>
#include <cmath>
#include <iomanip>

#include "Camera.h"
#include "utils.h"

#include "World.h"

using namespace std;

World::World()
	: renderListComplete(false)
{

}

World::~World()
{

}

#define CAMERA_CHUNK_RADIUS 5

void World::update()
{
	// Get camera position
	Vector3F pos = Camera::getInstance().getPosition();

	// Determine the chunk the camera is in.
	//Vector3I cameraChunkPos(0, 0, 0);

	pos = pos / Chunk::SIZE;
	Vector3I cameraChunkPos(round(pos.x), round(pos.y), round(pos.z));

	//cout << "camera at " << setprecision(2) << Camera::getInstance().getPosition() << " = chunk " << cameraChunkPos << endl;

	// Check for chunks to load, unload, generate and build renderList
	buildRenderList(cameraChunkPos);
}

void World::render()
{
	for(Chunk* c : renderList)
		c->render();
}

void World::buildRenderList(const Vector3I& cameraChunkPos)
{
	if(lastCameraChunk == cameraChunkPos && renderListComplete)
		return; // the camera chunk has not changed, no need to rebuild the render list

	// clear renderList
	renderList.clear();
	renderListComplete = true;

	// iterate a cube around the camera and check the chunk's distance to camera chunk to get a sphere around the camera
	for(int x = cameraChunkPos.x - CAMERA_CHUNK_RADIUS; x <= cameraChunkPos.x + CAMERA_CHUNK_RADIUS; x++)
		for(int y = cameraChunkPos.y - CAMERA_CHUNK_RADIUS; y <= cameraChunkPos.y + CAMERA_CHUNK_RADIUS; y++)
			for(int z = cameraChunkPos.z - CAMERA_CHUNK_RADIUS; z <= cameraChunkPos.z + CAMERA_CHUNK_RADIUS; z++)
			{
				Vector3I chunkPos(x, y, z);
				if(::distance(chunkPos, cameraChunkPos) > CAMERA_CHUNK_RADIUS)
					continue;

				Chunk* c = loader.get(chunkPos);
				if(c)
					renderList.push_back(c);
				else
					renderListComplete = false;
			}
}
