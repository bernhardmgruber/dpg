#pragma once

#include <array>
#include <optional>
#include <stdint.h>
#include <vector>

#include "geometry.h"
#include "mathlib.h"
#include "opengl/Buffer.h"

struct ChunkMemoryFootprint final {
	size_t densityValues;
	size_t densityValueSize;
	size_t triangles;
	size_t triangleSize;

	const size_t densityBytes() const {
		return densityValues * densityValueSize;
	}

	const size_t triangleBytes() const {
		return triangles * triangleSize;
	}

	const size_t totalBytes() const {
		return densityBytes() + triangleBytes();
	}
};

inline constexpr auto blockLength = 1.0f;
inline constexpr auto chunkResolution = 16;                      // The number of voxels along one axis. chunkResolution + 1 is the edge length of the density cube (a cube of voxels).
inline constexpr auto chunkSize = blockLength * chunkResolution; // The size of the chunk in world units.

using IdType = uint64_t;

IdType ChunkGridCoordinateToId(glm::ivec3 chunkGridCoord);
glm::ivec3 IdToChunkGridCoordinate(IdType id);

class Chunk final {
public:
	using DensityType = float;

	enum class VoxelType {
		SOLID,
		SURFACE,
		AIR
	};

	Chunk() = default;
	Chunk(IdType id);
	Chunk(glm::ivec3 chunkIndex);
	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;
	Chunk(Chunk&&) = default;
	Chunk& operator=(Chunk&&) = default;
	~Chunk();

	// Converts a voxel coordinate to a world coordinate.
	glm::vec3 toWorld(glm::vec3 voxel) const;
	glm::ivec3 toVoxelCoord(const glm::vec3& v) const;

	IdType getId() const;

	glm::ivec3 chunkIndex() const;
	glm::vec3 lower() const;

	/**
	* Categorizes the given position in world coordinates.
	*/
	VoxelType categorizeWorldPosition(const glm::vec3& pos) const;
	VoxelType categorizeVoxel(glm::ivec3 pos) const;

	/**
	* Renders the chunk.
	*/
	void render() const;
	void renderAuxiliary() const;

	/**
	* Called by ChunkManager
	*/
	void createBuffers();

	/**
	* Get the memory footprint of this chunk.
	*/
	ChunkMemoryFootprint getMemoryFootprint() const;

	DensityType densityAt(glm::ivec3 localIndex) const;
	std::array<DensityType, 8> densityCubeAt(glm::ivec3 localIndex) const;
	unsigned int caseIndexFromVoxel(std::array<DensityType, 8> values) const;

	auto aabb() const -> BoundingBox;
	auto voxelAabb(glm::ivec3 localIndex) const -> BoundingBox;
	auto fullTriangles() const -> std::vector<Triangle>;

	std::vector<DensityType> densities;
	std::vector<glm::uvec3> triangles;
	std::vector<RVertex> vertices;

private:
	IdType id{};
	glm::ivec3 index;

	std::optional<gl::Buffer> vertexBuffer;
	std::optional<gl::Buffer> indexBuffer;
};
