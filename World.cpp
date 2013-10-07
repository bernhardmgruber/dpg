#include <iostream>
#include <cmath>
#include <iomanip>

#include "Camera.h"
#include "utils.h"

#include "World.h"

using namespace std;

World::World()
{

}

World::~World()
{
	chunks.clear();
}

#define CAMERA_CHUNK_RADIUS 5

void World::update()
{
	//cout << "BEGIN world update" << endl;

	// Get camera position
	Vector3F pos = Camera::getInstance().getPosition();

	// Determine the chunk the camera is in.
	//Vector3I cameraChunkPos(0, 0, 0);

	pos = pos / Chunk::SIZE;
	Vector3I cameraChunkPos(round(pos.x), round(pos.y), round(pos.z));

	//cout << "camera at " << setprecision(2) << Camera::getInstance().getPosition() << " = chunk " << cameraChunkPos << endl;

	// Reset markers and clear renderList
	for(auto& tuple : chunks)
		get<1>(tuple.second) = false;
	renderList.clear();

	// Check for chunks to load, unload, generate and build renderList
	recursiveChunkCheck(cameraChunkPos, pos);

	//cout << "END world update (memory: " << sizeToString(getMemoryFootprint().totalBytes()) << ")" << endl;
}

void World::render()
{
	for(Chunk* c : renderList)
		c->render();
}

const ChunkMemoryFootprint World::getMemoryFootprint() const
{
	ChunkMemoryFootprint mem = {0};

	for(const auto& tuple : chunks)
	{
		const ChunkMemoryFootprint& cmem = get<0>(tuple.second)->getMemoryFootprint();
		mem.densityValues += cmem.densityValues;
		mem.densityValueSize = cmem.densityValueSize;
		mem.triangles += cmem.triangles;
		mem.triangleSize = cmem.triangleSize;

		//cout << tuple.first << " density " << sizeToString(cmem.densityBytes()) << " triangles " << sizeToString(cmem.triangleBytes()) << endl;
	}

	return mem;
}

void World::recursiveChunkCheck(const Vector3I& chunkPos, const Vector3F& cameraPos)
{
	if(::distance(Vector3F(chunkPos), cameraPos) > CAMERA_CHUNK_RADIUS)
		return;

	//cout << "Checking chunk " << chunkPos << " level " << level << endl;

	// try to find chunk
	Chunk* c;

	auto it = chunks.find(chunkPos);
	if(it == chunks.end())
	{
		// this chunk has not been loaded
		cout << "Chunk " << chunkPos << " not loaded" << endl;

		c = new Chunk(chunkPos);
		chunks[chunkPos] = make_tuple(c, true);

		const ChunkMemoryFootprint mem = getMemoryFootprint();
		cout << "memory: " << sizeToString(mem.totalBytes()) << "(density: " << sizeToString(mem.densityBytes()) << " triangles: " << sizeToString(mem.triangleBytes()) << ")" << endl;
	}
	else
	{
		// this chunk is loaded
		c = get<0>(it->second);

		if(get<1>(it->second) == true)
			return; // we have already visited this chunk

		get<1>(it->second) = true;
	}

	renderList.push_back(c);

	// recurse on neighbors
	Vector3I offsets[] = {Vector3I(1, 0, 0), Vector3I(-1, 0, 0), Vector3I(0, 1, 0), Vector3I(0, -1, 0), Vector3I(0, 0, 1), Vector3I(0, 0, -1)};
	for(Vector3I& offset : offsets)
	{
		Vector3I neighborCenter = c->getPosition() + offset;
		recursiveChunkCheck(neighborCenter, cameraPos);
	}
}
