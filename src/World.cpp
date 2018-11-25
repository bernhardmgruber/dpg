#include <cmath>
#include <iomanip>
#include <iostream>

#include "Camera.h"
#include "utils.h"

#include "World.h"

using namespace std;

World::World()
	: physics(loader), renderListComplete(false) {
}

World::~World() {
}

#define CAMERA_CHUNK_RADIUS 5

void World::update() {
	// Get camera position
	glm::ivec3 cameraChunkPos = physics.getChunkPos(camera.position);

	//cout << "camera at " << setprecision(2) << camera.position << " = chunk " << cameraChunkPos << endl;

	// Check for chunks to load, unload, generate and build renderList
	buildRenderList(cameraChunkPos);
}

void World::render() {
	for (Chunk* c : renderList)
		c->render();
}

void World::buildRenderList(const glm::ivec3& cameraChunkPos) {
	if (lastCameraChunk == cameraChunkPos && renderListComplete)
		return; // the camera chunk has not changed, no need to rebuild the render list

	// clear renderList
	renderList.clear();
	renderListComplete = true;

	// iterate a cube around the camera and check the chunk's distance to camera chunk to get a sphere around the camera
	for (int x = cameraChunkPos.x - CAMERA_CHUNK_RADIUS; x <= cameraChunkPos.x + CAMERA_CHUNK_RADIUS; x++)
		for (int y = cameraChunkPos.y - CAMERA_CHUNK_RADIUS; y <= cameraChunkPos.y + CAMERA_CHUNK_RADIUS; y++)
			for (int z = cameraChunkPos.z - CAMERA_CHUNK_RADIUS; z <= cameraChunkPos.z + CAMERA_CHUNK_RADIUS; z++) {
				glm::ivec3 chunkPos(x, y, z);
				if (distance(glm::vec3(chunkPos), glm::vec3(cameraChunkPos)) > CAMERA_CHUNK_RADIUS)
					continue;

				Chunk* c = loader.get(chunkPos);
				if (c)
					renderList.push_back(c);
				else
					renderListComplete = false;
			}
}
