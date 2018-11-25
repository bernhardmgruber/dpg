#pragma once

#include <array>
#include <stdint.h>
#include <vector>
#include <optional>

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

class Chunk final {
public:
	using DensityType = float;
	using IdType = uint64_t;

	enum class VoxelType {
		SOLID,
		SURFACE,
		AIR
	};

	static constexpr float SIZE = 1.0; // The size of the chunk in world units.
	static constexpr unsigned int RESOLUTION = 16; // The number of voxels along one axis. RESOLUTION + 1 is the edge length of the density cube (a cube of voxels).

	static IdType ChunkGridCoordinateToId(glm::ivec3 chunkGridCoord);
	static glm::ivec3 IdToChunkGridCoordinate(IdType id);

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
		float blockLength = SIZE / RESOLUTION;
		glm::vec3 v = blockLength * (voxel - 1.0f);
		return getWorldPosition() + v;
	}

	glm::uvec3 toVoxelCoord(const glm::vec3& v) const;

	const IdType getId() const;

	/**
	* Gets the position of the chunk's center in the chunk grid.
	*/
	const glm::ivec3 getChunkGridPositon() const;

	/**
	* Gets the position of the chunk's center in the world.
	*/
	const glm::vec3 getWorldPosition() const;

	/**
	* Categorizes the given position in world coordinates.
	*/
	VoxelType categorizeWorldPosition(const glm::vec3& pos) const;

	/**
	* Renders the chunk.
	*/
	void render() const;

	/**
	* Called by ChunkManager
	*/
	void createBuffers();

	/**
	* Get the memory footprint of this chunk.
	*/
	const ChunkMemoryFootprint getMemoryFootprint() const;

	DensityType voxelAt(unsigned int x, unsigned int y, unsigned int z) const;
	std::array<DensityType, 8> voxelCubeAt(unsigned int x, unsigned int y, unsigned int z) const;
	unsigned int caseIndexFromVoxel(std::array<DensityType, 8> values) const;

	std::vector<DensityType> densities;
	std::vector<glm::uvec3> triangles;
	std::vector<Vertex> vertices;

private:
	IdType id{};
	glm::ivec3 position;

	std::optional<gl::Buffer> vertexBuffer;
	std::optional<gl::Buffer> indexBuffer;
};
