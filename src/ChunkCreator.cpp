//#include <noise/noise.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include <glm/glm.hpp>

#include <mutex>
#include <unordered_map>

#include "ChunkCreator.h"
#include "globals.h"
#include "mathlib.h"

auto ChunkCreator::getChunk(const glm::ivec3& chunkPos) -> Chunk {
	Chunk c(chunkPos);

	//noise::module::Perlin perlin;
	//perlin.SetOctaveCount(5);
	//perlin.SetFrequency(0.3);

	auto noise = [](glm::vec3 pos) {
		//pos /= chunkResolution;

		return -pos.z + 0.5f + pos.x * 0.1f;
		// return (Chunk::DensityType)perlin.GetValue(pos.x, pos.y, pos.z);
		//return stb_perlin_fbm_noise3(pos.x, pos.y, pos.z, 2.0f, 0.5f, global::noise::octaves);
		//return stb_perlin_turbulence_noise3(pos.x, pos.y, pos.z, 2.0f, 0.5f, global::noise::octaves, 0, 0, 0);
	};

	const unsigned int size = chunkResolution + 1 + 2; // + 1 for corners and + 2 for marging
	c.densities.resize(size * size * size);

	for (unsigned int x = 0; x < size; x++) {
		for (unsigned int y = 0; y < size; y++) {
			for (unsigned int z = 0; z < size; z++) {
				glm::vec3 world = c.toWorld(glm::vec3{x, y, z} - glm::vec3{1});
				c.densities[z * size * size + y * size + x] = noise(world);
			}
		}
	}

	//cout << "Noise took " << timer.interval << " seconds" << endl;

	c.march();

	//cout << "Marching took " << timer.interval << " seconds" << endl;

	std::cout << "Created chunk:         " << chunkPos << '\n';

	//dumpTriangles("chunks/" + std::to_string(c.getId()) + "_triangles.ply", c.fullTriangles());
	//dumpLines("chunks/" + std::to_string(c.getId()) + "_aabb.ply", boxEdges(c.aabb()));

	return c;
}
