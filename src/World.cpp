#include <cmath>
#include <iomanip>
#include <iostream>

#include "Camera.h"
#include "geometry.h"
#include "globals.h"
#include "utils.h"

#include "World.h"

namespace {
	// based on http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-12-introduction-to-acceleration-structures/grid/
	class CellTraverser {
	public:
		CellTraverser(const World& world, Ray ray)
			: world(world) {
			cellIndex = world.getVoxelPos(ray.origin);

			for (auto i = 0; i < 3; i++) {
				step[i] = ray.direction[i] >= 0 ? 1 : -1;
				delta[i] = (ray.direction[i] >= 0 ? blockLength : -blockLength) / ray.direction[i];
				ts[i] = (ray.direction[i] >= 0 ? ((cellIndex[i] + 1) * blockLength - ray.origin[i]) : (ray.origin[i] - cellIndex[i] * blockLength)) / ray.direction[i];
			}
		}

		auto nextIndex() -> glm::ivec3 {
			const auto ci = cellIndex;

			const auto smallestIndex = [&] {
				if (ts.x < ts.y) {
					if (ts.x < ts.z)
						return 0;
					else
						return 2;
				} else {
					if (ts.y < ts.z)
						return 1;
					else
						return 2;
				}
			}();

			t = ts[smallestIndex];
			ts[smallestIndex] += delta[smallestIndex];
			cellIndex[smallestIndex] += step[smallestIndex];

			return ci;
		}

	private:
		const World& world;

		glm::ivec3 cellIndex;
		glm::ivec3 step;
		glm::vec3 delta;
		glm::vec3 ts;
		float t;
	};
}

World::World() {}

void World::update(Camera& camera) {
	// Get camera position
	glm::ivec3 cameraChunkPos = getChunkPos(camera.position);

	// Check for chunks to load, unload, generate and build renderList
	buildRenderList(cameraChunkPos);
}

void World::render() {
	for (Chunk* c : renderList)
		c->render();
}

void World::renderAuxiliary() {
	for (Chunk* c : renderList)
		c->renderAuxiliary();
}

void World::clearChunks() {
	chunks.clear();
}

auto World::trace(glm::vec3 start, glm::vec3 end) const -> glm::vec3 {
	if (start == end)
		return end;

	const auto startPos = getVoxelPos(start);
	const auto endPos = getVoxelPos(end);

	const auto delta = end - start;
	const auto maxT = length(delta);
	const auto ray = Ray{start, normalize(delta)};

	std::cout << "Tracing from " << start << " (" << startPos << ") to " << end << " (" << endPos << ")\n";

	CellTraverser t{*this, ray};
	while (true) {
		const glm::ivec3 blockIndex = t.nextIndex();
		std::cout << "    at block " << blockIndex << "\n";

		const auto chunkIndex = glm::ivec3{floor(glm::vec3{blockIndex} / (float)chunkResolution)}; // TODO: not very elegant

		const Chunk* chunk = chunks.get(chunkIndex);
		if (!chunk) {
			std::cout << "        no chunk, we stay at " << start << "\n";
			return start;
		}

		auto voxelIndex = blockIndex;
		voxelIndex.x %= chunkResolution;
		voxelIndex.y %= chunkResolution;
		voxelIndex.z %= chunkResolution;
		if (voxelIndex.x < 0) voxelIndex.x += chunkResolution;
		if (voxelIndex.y < 0) voxelIndex.y += chunkResolution;
		if (voxelIndex.z < 0) voxelIndex.z += chunkResolution;

		const auto cat = chunk->categorizeVoxel(voxelIndex);
		if (cat == Chunk::VoxelType::SOLID) {
			std::cerr << "tracing inside solid block\n";
			return start;
		} else if (cat == Chunk::VoxelType::SURFACE) {
			// TODO: super inefficient, only intersect against voxel triangles
			if (const auto t = intersectForDistance(ray, chunk->fullTriangles()); *t && *t > 0 && *t <= maxT) {
				const auto hit = ray.origin + ray.direction * (*t - 0.01f); // move back by some delta to avoid getting stuck in the surface
				std::cout << "        surface intersection at " << hit << "\n";
				return hit;
			}
		} else {
			assert(cat == Chunk::VoxelType::AIR);
			std::cout << "        air\n";
		}

		if (blockIndex == endPos)
			break;
	}

	std::cout << "        no intersection, reaching " << end << "\n";

	return end;
}

auto World::categorizeWorldPosition(const glm::vec3& pos) const -> Chunk::VoxelType {
	const glm::ivec3 chunkPos = getChunkPos(pos);
	const Chunk* chunk = chunks.get(chunkPos);
	return chunk->categorizeWorldPosition(pos);
}

glm::vec3 World::getNearestNonSolidPos(const glm::vec3& pos, BoundingBox& box) const {
	return pos;
}

glm::ivec3 World::getChunkPos(const glm::vec3& pos) const {
	glm::vec3 chunkPos = pos / chunkSize;
	return glm::ivec3(floor(chunkPos));
}

glm::ivec3 World::getVoxelPos(const glm::vec3& pos) const {
	glm::vec3 blockPos = pos / blockLength;
	return glm::ivec3(floor(blockPos));
}

void World::buildRenderList(const glm::ivec3& cameraChunkPos) {
	if (lastCameraChunk == cameraChunkPos && renderListComplete)
		return; // the camera chunk has not changed, no need to rebuild the render list

	// clear renderList
	renderList.clear();
	renderListComplete = true;

	// iterate a cube around the camera and check the chunk's distance to camera chunk to get a sphere around the camera
	for (int x = cameraChunkPos.x - global::CAMERA_CHUNK_RADIUS; x <= cameraChunkPos.x + global::CAMERA_CHUNK_RADIUS; x++)
		for (int y = cameraChunkPos.y - global::CAMERA_CHUNK_RADIUS; y <= cameraChunkPos.y + global::CAMERA_CHUNK_RADIUS; y++)
			for (int z = cameraChunkPos.z - global::CAMERA_CHUNK_RADIUS; z <= cameraChunkPos.z + global::CAMERA_CHUNK_RADIUS; z++) {
				glm::ivec3 chunkPos(x, y, z);
				if (distance(glm::vec3(chunkPos), glm::vec3(cameraChunkPos)) > global::CAMERA_CHUNK_RADIUS)
					continue;

				if (Chunk* c = chunks.get(chunkPos))
					renderList.push_back(c);
				else
					renderListComplete = false;
			}
}
