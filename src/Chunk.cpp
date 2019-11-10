#include "Chunk.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <unordered_map>

#include "globals.h"
#include "tables.inc"

using namespace std;

namespace {
	constexpr auto initialTriangleMapSize = 3000;

	void drawBoxEdges(BoundingBox box) {
		const auto edges = boxEdges(box);

		glBegin(GL_LINES);
		for (const auto& e : edges)
			for (const auto& v : e)
				glVertex3fv(glm::value_ptr(v));
		glEnd();
	}

	glm::vec3 gradient(Chunk& c, glm::ivec3 v) {
		glm::vec3 grad;
		grad.x = c.densityAt(v + glm::ivec3{1, 0, 0}) - c.densityAt(v - glm::ivec3{1, 0, 0});
		grad.y = c.densityAt(v + glm::ivec3{0, 1, 0}) - c.densityAt(v - glm::ivec3{0, 1, 0});
		grad.z = c.densityAt(v + glm::ivec3{0, 0, 1}) - c.densityAt(v - glm::ivec3{0, 0, 1});
		return normalize(grad);
	}
}

IdType ChunkGridCoordinateToId(glm::ivec3 index) {
	constexpr uint32_t mask = 0x001FFFFF; // 21 bit
	return (((IdType)(index.x & mask)) << 42) | (((IdType)(index.y & mask)) << 21) | (((IdType)(index.z & mask)) << 0);
}

glm::ivec3 IdToChunkGridCoordinate(IdType id) {
	uint32_t mask = 0x001FFFFF; // 21 bit
	return glm::ivec3((id >> 42) & mask, (id >> 21) & mask, (id >> 0) & mask);
}

Chunk::Chunk(IdType id)
	: id(id), index(IdToChunkGridCoordinate(id)) {}

Chunk::Chunk(glm::ivec3 index)
	: id(ChunkGridCoordinateToId(index)), index(index) {}

Chunk::~Chunk() = default;

glm::vec3 Chunk::toWorld(glm::vec3 voxel) const {
	return lower() + voxel;
}

glm::ivec3 Chunk::toVoxelCoord(const glm::vec3& v) const {
	glm::vec3 rel = v - lower();
	assert(rel.x >= 0 && rel.x < 16);
	assert(rel.y >= 0 && rel.y < 16);
	assert(rel.z >= 0 && rel.z < 16);
	return glm::ivec3{rel};
	}

IdType Chunk::getId() const {
	return id;
}

glm::ivec3 Chunk::chunkIndex() const {
	return index;
}

glm::vec3 Chunk::lower() const {
	return glm::vec3{index * chunkResolution};
}

Chunk::VoxelType Chunk::categorizeWorldPosition(const glm::vec3& pos) const {
	return categorizeVoxel(toVoxelCoord(pos));
}

Chunk::VoxelType Chunk::categorizeVoxel(glm::ivec3 pos) const {
	assert(pos.x >= 0 && pos.x < chunkResolution);
	assert(pos.y >= 0 && pos.y < chunkResolution);
	assert(pos.z >= 0 && pos.z < chunkResolution);

	const auto caseIndex = caseIndexFromVoxel(densityCubeAt(pos));

	if (caseIndex == 255) return VoxelType::SOLID;
	if (caseIndex == 0) return VoxelType::AIR;
	return VoxelType::SURFACE;
}

void Chunk::march() {
	vertices.clear();
	triangles.clear();

	std::unordered_map<glm::vec3, unsigned int> vertexMap(initialTriangleMapSize);

	glm::ivec3 bi;
	for (bi.x = 0; bi.x < chunkResolution; bi.x++) {
		for (bi.y = 0; bi.y < chunkResolution; bi.y++) {
			for (bi.z = 0; bi.z < chunkResolution; bi.z++) {
				const std::array<Chunk::DensityType, 8> values = densityCubeAt(bi);

				const auto caseIndex = caseIndexFromVoxel(values);
				if (caseIndex == 255)
					continue; // solid voxel
				if (caseIndex == 0)
					continue; // air voxel

				const int numTriangles = case_to_numpolys[caseIndex];

				// for each triangle of the cube
				for (int t = 0; t < numTriangles; t++) {
					glm::ivec3 tri;

					// for each edge of the cube a triangle vertex is on
					for (int e = 0; e < 3; e++) {
						const int edgeIndex = edge_connect_list[caseIndex][t][e];

						const auto [value1, value2, vec1, vec2] = [&]() -> std::tuple<Chunk::DensityType, Chunk::DensityType, glm::ivec3, glm::ivec3> {
							switch (edgeIndex) {
								case 0: return {values[0], values[1], bi + glm::ivec3(0, 0, 0), bi + glm::ivec3(0, 0, 1)};
								case 1: return {values[1], values[2], bi + glm::ivec3(0, 0, 1), bi + glm::ivec3(1, 0, 1)};
								case 2: return {values[2], values[3], bi + glm::ivec3(1, 0, 1), bi + glm::ivec3(1, 0, 0)};
								case 3: return {values[3], values[0], bi + glm::ivec3(1, 0, 0), bi + glm::ivec3(0, 0, 0)};
								case 4: return {values[4], values[5], bi + glm::ivec3(0, 1, 0), bi + glm::ivec3(0, 1, 1)};
								case 5: return {values[5], values[6], bi + glm::ivec3(0, 1, 1), bi + glm::ivec3(1, 1, 1)};
								case 6: return {values[6], values[7], bi + glm::ivec3(1, 1, 1), bi + glm::ivec3(1, 1, 0)};
								case 7: return {values[7], values[4], bi + glm::ivec3(1, 1, 0), bi + glm::ivec3(0, 1, 0)};
								case 8: return {values[0], values[4], bi + glm::ivec3(0, 0, 0), bi + glm::ivec3(0, 1, 0)};
								case 9: return {values[1], values[5], bi + glm::ivec3(0, 0, 1), bi + glm::ivec3(0, 1, 1)};
								case 10: return {values[2], values[6], bi + glm::ivec3(1, 0, 1), bi + glm::ivec3(1, 1, 1)};
								case 11: return {values[3], values[7], bi + glm::ivec3(1, 0, 0), bi + glm::ivec3(1, 1, 0)};
								default: std::terminate();
							}
						}();

						const glm::vec3 vertex = interpolate(value1, value2, glm::vec3(vec1), glm::vec3(vec2));

						// lookup this vertex
						if (auto it = vertexMap.find(vertex); it != vertexMap.end())
							tri[e] = it->second;
						else {
							// calculate a new one
							RVertex v;
							v.position = toWorld(vertex);

							// the gradient points towards higher densities (it points into the solidness), therefore invert the normal
							const glm::vec3 g1 = gradient(*this, vec1);
							const glm::vec3 g2 = gradient(*this, vec2);
							v.normal = -normalize(interpolate(value1, value2, g1, g2));

							tri[e] = (unsigned int)vertices.size();
							vertices.push_back(v);

							vertexMap[v.position] = tri[e];
						}
					}

					// reorient triangles
					std::swap(tri[1], tri[2]);

					triangles.push_back(tri);
				}
			}
		}
	}
}

void Chunk::render() const {
	if (global::showTriangles) {
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.value().id());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.value().id());

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (const GLvoid*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (const GLvoid*)sizeof(glm::vec3));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawElements(GL_TRIANGLES, (GLsizei)triangles.size() * 3, GL_UNSIGNED_INT, (const GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
}

void Chunk::renderAuxiliary() const {
	if (global::showTriangleNormals) {
		glColor3f(1.0, 1.0, 0.0);
		glBegin(GL_LINES);
		for (auto t : triangles) {
			const auto v0 = vertices[t[0]].position;
			const auto v1 = vertices[t[1]].position;
			const auto v2 = vertices[t[2]].position;
			const auto normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
			auto pos = (v0 + v1 + v2) / 3.0f;

			glVertex3fv((float*)&pos);
			pos = pos + (normal * 0.5f);
			glVertex3fv((float*)&pos);
		}
		glEnd();
	}

	if (global::showVertexNormals) {
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINES);
		for (auto t : triangles) {
			for (int i = 0; i < 3; i++) {
				RVertex vert = vertices[t[i]];
				glm::vec3 pos = vert.position;
				glm::vec3 normal = vert.normal;


				glVertex3fv((float*)&pos);
				pos = pos + (normal * 0.5f);
				glVertex3fv((float*)&pos);
			}
		}
		glEnd();
	}

	if (global::showChunks) {
		glColor3f(1.0, 0.0, 0.0);
		drawBoxEdges(aabb());
	}

	if (global::showVoxels) {
		const auto chunkLower = lower();

		glColor3f(1, 0, 0);
		glm::ivec3 bi;
		for (bi.x = 0; bi.x < chunkResolution; bi.x++) {
			for (bi.y = 0; bi.y < chunkResolution; bi.y++) {
				for (bi.z = 0; bi.z < chunkResolution; bi.z++) {
					const auto cat = categorizeVoxel(bi);
					if (cat != VoxelType::SURFACE)
						continue;
					drawBoxEdges(voxelAabb(bi));
				}
			}
		}
	}
}

ChunkMemoryFootprint Chunk::getMemoryFootprint() const {
	const unsigned int size = chunkResolution + 1 + 2; // + 1 for corners and + 2 for marging

	ChunkMemoryFootprint mem{};
	mem.densityValues = size * size * size;
	mem.densityValueSize = sizeof(DensityType);
	mem.triangles = triangles.size();
	mem.triangleSize = sizeof(Triangle);
	return mem;
}

void Chunk::createBuffers() {
	vertexBuffer.emplace();
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.value().id());
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);

	indexBuffer.emplace();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.value().id());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(triangles[0]), triangles.data(), GL_STATIC_DRAW);
}

float Chunk::densityAt(glm::ivec3 localIndex) const {
	// -1 and chunkResolution access border densities from neighboring chunks
	assert(localIndex.x >= -1 && localIndex.x <= chunkResolution + 1);
	assert(localIndex.y >= -1 && localIndex.y <= chunkResolution + 1);
	assert(localIndex.z >= -1 && localIndex.z <= chunkResolution + 1);
	localIndex += 1;

	constexpr auto side = chunkResolution + 1 + 2;
	return densities[localIndex.z * side * side + localIndex.y * side + localIndex.x];
}

std::array<Chunk::DensityType, 8> Chunk::densityCubeAt(glm::ivec3 localIndex) const {
	std::array<DensityType, 8> values;
	values[0] = densityAt(localIndex + glm::ivec3{0, 0, 0});
	values[1] = densityAt(localIndex + glm::ivec3{0, 0, 1});
	values[2] = densityAt(localIndex + glm::ivec3{1, 0, 1});
	values[3] = densityAt(localIndex + glm::ivec3{1, 0, 0});
	values[4] = densityAt(localIndex + glm::ivec3{0, 1, 0});
	values[5] = densityAt(localIndex + glm::ivec3{0, 1, 1});
	values[6] = densityAt(localIndex + glm::ivec3{1, 1, 1});
	values[7] = densityAt(localIndex + glm::ivec3{1, 1, 0});
	return values;
}

unsigned int Chunk::caseIndexFromVoxel(std::array<DensityType, 8> values) const {
	unsigned int caseIndex = 0;

	if (values[0] > 0) caseIndex |= 0x01;
	if (values[1] > 0) caseIndex |= 0x02;
	if (values[2] > 0) caseIndex |= 0x04;
	if (values[3] > 0) caseIndex |= 0x08;
	if (values[4] > 0) caseIndex |= 0x10;
	if (values[5] > 0) caseIndex |= 0x20;
	if (values[6] > 0) caseIndex |= 0x40;
	if (values[7] > 0) caseIndex |= 0x80;

	return caseIndex;
}

auto Chunk::aabb() const -> BoundingBox {
	const auto l = lower();
	return {l, l + (float)chunkResolution};
}

auto Chunk::voxelAabb(glm::ivec3 localIndex) const -> BoundingBox {
	const auto l = lower() + glm::vec3(localIndex);
	return {l, l + 1.0f};
}

auto Chunk::fullTriangles() const -> std::vector<Triangle> {
	std::vector<Triangle> result;
	for (const auto& t : triangles) {
		result.emplace_back(
			vertices[t.x].position,
			vertices[t.y].position,
			vertices[t.z].position);
	}
	return result;
}
