#pragma once

#include "gl.h"
#include <vector>
#include <array>

#include "mathlib.h"

struct ChunkMemoryFootprint
{
    size_t densityValues;
    size_t densityValueSize;
    size_t triangles;
    size_t triangleSize;

    const size_t densityBytes() const
    {
        return densityValues * densityValueSize;
    }

    const size_t triangleBytes() const
    {
        return triangles * triangleSize;
    }

    const size_t totalBytes() const
    {
        return densityBytes() + triangleBytes();
    }
};

class Chunk
{
public:
    typedef float DensityType;

    enum class BlockType
    {
        SOLID,
        SURFACE,
        AIR
    };

    /**
    * The size of the chunk
    */
    static const float SIZE;

    /**
    * The number of sub cubes along one axis.
    * RESOLUTION + 1 is the edge length of the density cube.
    */
    static const unsigned int RESOLUTION;

    /**
    * ctor
    * Generates a new chunk for the given chunk position
    */
    Chunk(Vector3I position);

    /**
    * dtor
    */
    ~Chunk();

    /**
    * Converts a density value coordinate to a world coordinate.
    */
    template<typename T>
    Vector3F toWorld(T x, T y, T z) const
    {
        Vector3F v;
        float blockLength = SIZE / RESOLUTION;
        v.x = blockLength * ((float)x - 1.0f);
        v.y = blockLength * ((float)y - 1.0f);
        v.z = blockLength * ((float)z - 1.0f);
        return getWorldPosition() + v;
    }

    /**
    * Converts a density value coordinate to a world coordinate.
    */
    template<typename T>
    Vector3<T> toWorld(Vector3<T> v) const
    {
        return toWorld<T>(v.x, v.y, v.z);
    }
	
    Vector3UI toDensityBlockCoord(const Vector3F& v) const;

    /**
    * Gets the position of the chunk's center in the voxel grid.
    */
    const Vector3I getVoxelPosition() const;

    /**
    * Gets the position of the chunk's center in the world.
    */
    const Vector3F getWorldPosition() const;

    /**
    * Categorizes the given position in world coordinates.
    */
    BlockType categorizeWorldPosition(const Vector3F& pos) const;

    /**
    * Renders the chunk.
    */
    void render() const;

    /**
    * Get the memory footprint of this chunk.
    */
    const ChunkMemoryFootprint getMemoryFootprint() const;

    /**
    * Called by ChunkLoader
    */
    void createBuffers();

private:
    Vector3I position;

    DensityType* densities;

    std::vector<Vector3UI> triangles;

    std::vector<Vertex> vertices;

    bool buffersInitialized;

    GLuint vertexBuffer;
    GLuint indexBuffer;

    //
    // marching cubes and density functions
    //

    template<typename T>
    Vector3F interpolate(DensityType da, DensityType db, Vector3<T> va, Vector3<T> vb) const
    {
        Vector3F result;

        DensityType part = fabs(da) / (fabs(da) + fabs(db));

        result.x = va.x + (vb.x - va.x) * part;
        result.y = va.y + (vb.y - va.y) * part;
        result.z = va.z + (vb.z - va.z) * part;

        return result;
    }

    inline DensityType blockAt(DensityType* block, unsigned int x, unsigned int y, unsigned int z) const;

    Vector3F getNormal(DensityType* block, const Vector3UI& v) const;

    inline std::array<DensityType, 8> getDensityBlockAt(DensityType* block, unsigned int x, unsigned int y, unsigned int z) const;

    inline unsigned int getCaseIndexFromDensityBlock(std::array<DensityType, 8> values) const;

    void marchChunk(DensityType* block);

};
