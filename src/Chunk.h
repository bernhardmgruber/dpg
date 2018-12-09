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
inline constexpr auto chunkResolution = 16u;                     // The number of voxels along one axis. chunkResolution + 1 is the edge length of the density cube (a cube of voxels).
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
	Chunk(glm::ivec3 chunkGridCoord);
	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;
	Chunk(Chunk&&) = default;
	Chunk& operator=(Chunk&&) = default;
	~Chunk();

	// Converts a voxel coordinate to a world coordinate.
	glm::vec3 toWorld(glm::vec3 voxel) const {
		float blockLength = chunkSize / chunkResolution;
		glm::vec3 v = blockLength * (voxel - 1.0f);
		return getWorldPosition() + v;
	}

	glm::uvec3 toVoxelCoord(const glm::vec3& v) const;

	IdType getId() const;

	/**
	* Gets the position of the chunk's center in the chunk grid.
	*/
	glm::ivec3 getChunkGridPositon() const;

	/**
	* Gets the position of the chunk's center in the world.
	*/
	glm::vec3 getWorldPosition() const;

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

	DensityType voxelAt(unsigned int x, unsigned int y, unsigned int z) const;
	std::array<DensityType, 8> voxelCubeAt(unsigned int x, unsigned int y, unsigned int z) const;
	unsigned int caseIndexFromVoxel(std::array<DensityType, 8> values) const;

	auto aabb() const -> BoundingBox;
	auto fullTriangles() const -> std::vector<Triangle>;

	std::vector<DensityType> densities;
	std::vector<glm::uvec3> triangles;
	std::vector<RVertex> vertices;

private:
	IdType id{};
	glm::ivec3 position;

	std::optional<gl::Buffer> vertexBuffer;
	std::optional<gl::Buffer> indexBuffer;
};
