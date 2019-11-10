#include <Windows.h>

#include <cmath>
#include <iomanip>
#include <iostream>

#include "Camera.h"
#include "geometry.h"
#include "globals.h"
#include "utils.h"

#include "World.h"


namespace {
	auto voxelPos(glm::vec3 pos, float voxelLength) -> glm::ivec3 {
		auto chunkPos = pos / voxelLength;
		return glm::ivec3(floor(chunkPos));
	}

	// based on http://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/grid
	class CellTraverser {
	public:
		CellTraverser(float voxelLength, Ray ray)
			: cellIndex(voxelPos(ray.origin, voxelLength)) {
			for (auto i = 0; i < 3; i++) {
				step[i] = ray.direction[i] >= 0 ? 1 : -1;
				delta[i] = step[i] * voxelLength / ray.direction[i];
				const auto lower = cellIndex[i] * voxelLength;
				const auto upper = (cellIndex[i] + 1) * voxelLength;
				nextT[i] = ((ray.direction[i] >= 0 ? upper : lower) - ray.origin[i]) / ray.direction[i];
			}
		}

		auto nextIndex() -> glm::ivec3 {
			const auto ci = cellIndex;

			const auto smallestIndex = [&] {
				if (nextT.x < nextT.y) {
					if (nextT.x < nextT.z)
						return 0;
					else
						return 2;
				} else {
					if (nextT.y < nextT.z)
						return 1;
					else
						return 2;
				}
			}();

			t = nextT[smallestIndex];
			nextT[smallestIndex] += delta[smallestIndex];
			cellIndex[smallestIndex] += step[smallestIndex];

			return ci;
		}

		auto distanceFromOrigin() const -> float {
			return t;
		}

	private:
		glm::ivec3 cellIndex;
		glm::ivec3 step;
		glm::vec3 delta;
		glm::vec3 nextT;
		float t = 0;
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

static auto interpolateLinear(float coord, float v0, float v1) {
	return v0 * (1 - coord) + v1 * coord;
}

// v10 -- v11
//  |      |
// v00 -- v01
static auto interpolateBilinear(glm::vec2 coord, float v00, float v01, float v11, float v10) {
	const auto x1 = interpolateLinear(coord.x, v00, v01);
	const auto x2 = interpolateLinear(coord.x, v10, v11);
	return interpolateLinear(coord.y, x1, x2);
}

static auto interpolateTrilinear(glm::vec3 coord, std::array<float, 8> densities) {
	const auto y1 = interpolateBilinear({coord.z, coord.x}, densities[0], densities[1], densities[2], densities[3]);
	const auto y2 = interpolateBilinear({coord.z, coord.x}, densities[4], densities[5], densities[6], densities[7]);
	return interpolateLinear(coord.y, y1, y2);
}

static auto gradient(glm::vec3 coord, std::array<float, 8> d) {
	return glm::vec3{
		interpolateBilinear({coord.y, coord.z}, d[3] - d[0], d[7] - d[4], d[6] - d[5], d[2] - d[1]),
		interpolateBilinear({coord.x, coord.z}, d[4] - d[0], d[7] - d[3], d[6] - d[2], d[5] - d[1]),
		interpolateBilinear({coord.x, coord.y}, d[1] - d[0], d[2] - d[3], d[6] - d[7], d[5] - d[4])};
}

constexpr auto hitAtStartEpsilon = 0.001f;

auto World::trace(const glm::vec3 start, const glm::vec3 end, bool dump) const -> TraceResult {
	if (start == end)
		return {end, false};

	const auto startPos = getVoxelPos(start);
	const auto endPos = getVoxelPos(end);

	const auto delta = end - start;
	const auto maxT = length(delta);
	const auto ray = Ray{start, normalize(delta)};

	//dump = GetKeyState('D') & 0x8000;

	static auto counter = 0;
	counter++;
	if (dump)
		dumpLines("trace" + std::to_string(counter) + "/line.ply", std::vector{Line{start, end}});

	std::cout << "Tracing from " << start << " to " << end << "\n";

	int i = 0;
	CellTraverser traverser{1.0f, ray};
	while (true) {
		i++;

		const glm::ivec3 voxelIndex = traverser.nextIndex();
		std::cout << "    at voxel " << voxelIndex << "\n";

		const Chunk* chunk = chunks.get(voxelIndex / chunkResolution);
		if (!chunk) {
			//std::cout << "        no chunk, we stay at " << start << "\n";
			return {start, false};
		}

		auto localIndex = voxelIndex;
		localIndex.x %= chunkResolution;
		localIndex.y %= chunkResolution;
		localIndex.z %= chunkResolution;
		if (localIndex.x < 0) localIndex.x += chunkResolution;
		if (localIndex.y < 0) localIndex.y += chunkResolution;
		if (localIndex.z < 0) localIndex.z += chunkResolution;

		if (dump) {
			const auto& l = chunk->lower();
			dumpLines("trace" + std::to_string(counter) + "/aabb_" + std::to_string(i) + ".ply", boxEdges({l + glm::vec3{voxelIndex}, l + glm::vec3{voxelIndex} + 1.0f}));
		}

		const auto densities = chunk->densityCubeAt(localIndex);
		const auto case_ = chunk->caseIndexFromVoxel(densities);
		const auto cat = chunk->categorizeVoxel(localIndex);
		std::cout << "    cat " << (int)cat << " case " << case_ << " densities: " << densities[0] << ", " << densities[1] << ", " << densities[2] << ", " << densities[3] << ", " << densities[4] << ", " << densities[5] << ", " << densities[6] << ", " << densities[7] << "\n";
		if (cat == Chunk::VoxelType::SOLID) {
			// this is problematic, because we must have missed a surface intersection
			std::cerr << "    ERROR: tracing inside solid block\n";
			return {start, false};
		} else if (cat == Chunk::VoxelType::SURFACE) {
			const auto box = chunk->voxelAabb(localIndex);
			if (const auto hitDepths = intersectBox(box, ray)) {
				const auto entry = ray.origin + ray.direction * hitDepths->first;
				const auto exit = ray.origin + ray.direction * hitDepths->second;
				const auto entryCoord = clamp(entry - box.lower, {glm::vec3{0}, glm::vec3{1}});
				const auto exitCoord = clamp(exit - box.lower, {glm::vec3{0}, glm::vec3{1}});
				const auto densities = chunk->densityCubeAt(localIndex);
				const auto entryDensity = interpolateTrilinear(entryCoord, densities);
				const auto exitDensity = interpolateTrilinear(exitCoord, densities);
				std::cout << "        entry/exit at " << entry << " " << exit << " densities " << entryDensity << " " << exitDensity << "\n";
				const auto t = interpolate(entryDensity, exitDensity, hitDepths->first, hitDepths->second);
				if (t > maxT)
					return {end, false};
				const auto hit = ray.origin + ray.direction * t;

				if (t < hitAtStartEpsilon) {
					// if we hit the ground at the start, but we are jumping away, continue
					const auto hitCoord = clamp(hit - box.lower, {glm::vec3{0}, glm::vec3{1}});
					const auto hitNormal = -normalize(gradient(hitCoord, densities));
					std::cout << "        hit at start, t " << t << " normal " << hitNormal << " ray " << ray.direction << "\n";
					if (dot(hitNormal, ray.direction) > 0)
						continue;
				}

				// TODO: compute slide off vector and continue tracing
				return {hit, true};
			}
		} else {
			assert(cat == Chunk::VoxelType::AIR);
			//std::cout << "        air\n";
		}

		if (voxelIndex == endPos || traverser.distanceFromOrigin() > maxT)
			break;
	}

	//std::cout << "        no intersection, reaching " << end << "\n";

	return {end, false};
}

auto World::categorizeWorldPosition(const glm::vec3& pos) const -> Chunk::VoxelType {
	const glm::ivec3 chunkPos = getChunkPos(pos);
	if (const Chunk* chunk = chunks.get(chunkPos))
		return chunk->categorizeWorldPosition(pos);
	return Chunk::VoxelType::UNKNOWN;
}

glm::vec3 World::getNearestNonSolidPos(const glm::vec3& pos, BoundingBox& box) const {
	return pos;
}

glm::ivec3 World::getChunkPos(const glm::vec3& pos) const {
	return voxelPos(pos, chunkResolution);
}

glm::ivec3 World::getVoxelPos(const glm::vec3& pos) const {
	return voxelPos(pos, 1.0f);
}

void World::buildRenderList(const glm::ivec3& cameraChunkPos) {
	if (lastCameraChunk == cameraChunkPos && renderListComplete)
		return; // the camera chunk has not changed, no need to rebuild the render list

	lastCameraChunk = cameraChunkPos;

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
