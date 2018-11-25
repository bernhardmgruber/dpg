#include <iostream>
#include <unordered_map>

#include "Timer.h"
#include "globals.h"

#include "Chunk.h"

using namespace std;

Chunk::IdType Chunk::ChunkGridCoordinateToId(glm::ivec3 chunkGridCoord) {
	uint32_t mask = 0x001FFFFF; // 21 bit

	return (((IdType)(chunkGridCoord.x & mask)) << 42) | (((IdType)(chunkGridCoord.y & mask)) << 21) | (((IdType)(chunkGridCoord.z & mask)) << 0);
}

glm::ivec3 Chunk::IdToChunkGridCoordinate(IdType id) {
	uint32_t mask = 0x001FFFFF; // 21 bit
	return glm::ivec3((id >> 42) & mask, (id >> 21) & mask, (id >> 0) & mask);
}

Chunk::Chunk(IdType id)
	: id(id), position(IdToChunkGridCoordinate(id)) {}

Chunk::Chunk(glm::ivec3 chunkGridCoord)
	: id(ChunkGridCoordinateToId(chunkGridCoord)), position(chunkGridCoord) {}

Chunk::~Chunk() = default;

glm::uvec3 Chunk::toVoxelCoord(const glm::vec3& v) const {
	glm::vec3 rel = (v - getWorldPosition()) / SIZE * (float)RESOLUTION;

	glm::uvec3 blockCoord((unsigned int)rel.x, (unsigned int)rel.y, (unsigned int)rel.z);

	assert(rel.x > 0 && rel.x < 16);
	assert(rel.y > 0 && rel.y < 16);
	assert(rel.z > 0 && rel.z < 16);

	return blockCoord;
}

const Chunk::IdType Chunk::getId() const {
	return id;
}

const glm::ivec3 Chunk::getChunkGridPositon() const {
	return position;
}

const glm::vec3 Chunk::getWorldPosition() const {
	glm::vec3 v;
	v.x = position.x * SIZE - SIZE / 2.0f;
	v.y = position.y * SIZE - SIZE / 2.0f;
	v.z = position.z * SIZE - SIZE / 2.0f;
	return v;
}

Chunk::VoxelType Chunk::categorizeWorldPosition(const glm::vec3& pos) const {
	const glm::uvec3 blockCoord = toVoxelCoord(pos);
	unsigned int caseIndex = caseIndexFromVoxel(voxelCubeAt(blockCoord.x, blockCoord.y, blockCoord.z));

	if (caseIndex == 255) return VoxelType::SOLID;
	if (caseIndex == 0) return VoxelType::AIR;
	return VoxelType::SURFACE;
}

void Chunk::render() const {
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

	if (global::normals) {
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINES);
		for (auto t : triangles) {
			for (int i = 0; i < 3; i++) {
				Vertex vert = vertices[t[i]];
				glm::vec3 pos = vert.position;
				glm::vec3 normal = vert.normal;


				glVertex3fv((float*)&pos);
				pos = pos + (normal * 0.05f);
				glVertex3fv((float*)&pos);
			}
		}
		glEnd();
	}
}

const ChunkMemoryFootprint Chunk::getMemoryFootprint() const {
	const unsigned int size = RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging

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


float Chunk::voxelAt(unsigned int x, unsigned int y, unsigned int z) const {
	assert(x < (RESOLUTION + 1 + 2));
	assert(y < (RESOLUTION + 1 + 2));
	assert(z < (RESOLUTION + 1 + 2));
	return densities[x * (RESOLUTION + 1 + 2) * (RESOLUTION + 1 + 2) + y * (RESOLUTION + 1 + 2) + z];
}

std::array<Chunk::DensityType, 8> Chunk::voxelCubeAt(unsigned int x, unsigned int y, unsigned int z) const {
	std::array<DensityType, 8> values;
	values[0] = voxelAt(x, y, z);
	values[1] = voxelAt(x, y, z + 1);
	values[2] = voxelAt(x + 1, y, z + 1);
	values[3] = voxelAt(x + 1, y, z);
	values[4] = voxelAt(x, y + 1, z);
	values[5] = voxelAt(x, y + 1, z + 1);
	values[6] = voxelAt(x + 1, y + 1, z + 1);
	values[7] = voxelAt(x + 1, y + 1, z);
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