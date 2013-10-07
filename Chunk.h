#ifndef CHUNK_H
#define CHUNK_H

#include <vector>

#include "mathlib.h"

class Chunk
{
public:
	/**
	* The size of the chunk
	*/
	static const float SIZE;

	/**
	* The size of the cube of density values for this chunk
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
	Vector3F toWorld(T x, T y, T z)
	{
		Vector3F v;
		v.x = position.x * SIZE - SIZE / 2.0f + SIZE / RESOLUTION * (x - 1);
		v.y = position.y * SIZE - SIZE / 2.0f + SIZE / RESOLUTION * (y - 1);
		v.z = position.z * SIZE - SIZE / 2.0f + SIZE / RESOLUTION * (z - 1);
		return v;
	}

	/**
	* Converts a density value coordinate to a world coordinate.
	*/
	template<typename T>
	Vector3<T> toWorld(Vector3<T> v)
	{
		return toWorld<T>(v.x, v.y, v.z);
	}

	Vector3I getPosition();

	void render();

private:
	Vector3I position;

	float* densities;

	std::vector<Triangle> triangles;

	/** Set to true by World::RecursiceChunkCheck to avoid infinite recursion. */
	bool marked;

	friend void MarchChunk(Chunk& c, float* block);
	friend class World;
};

#endif // CHUNK_H
