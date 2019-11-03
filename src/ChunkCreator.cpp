//#include <noise/noise.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include <glm/glm.hpp>

#include <mutex>
#include <unordered_map>

#include "ChunkCreator.h"
#include "globals.h"

#include "tables.inc"

namespace {
	constexpr auto initialTriangleMapSize = 3000;

	glm::vec3 interpolate(float da, float db, glm::vec3 va, glm::vec3 vb) {
		const float part = fabs(da) / (fabs(da) + fabs(db));
		return va + (vb - va) * part;
	}

	glm::vec3 gradient(Chunk& c, glm::ivec3 v) {
		glm::vec3 grad;
		grad.x = c.densityAt(v + glm::ivec3{1, 0, 0}) - c.densityAt(v - glm::ivec3{1, 0, 0});
		grad.y = c.densityAt(v + glm::ivec3{0, 1, 0}) - c.densityAt(v - glm::ivec3{0, 1, 0});
		grad.z = c.densityAt(v + glm::ivec3{0, 0, 1}) - c.densityAt(v - glm::ivec3{0, 0, 1});
		return normalize(grad);
	}

	void marchChunk(Chunk& c) {
		std::unordered_map<glm::vec3, unsigned int> vertexMap(initialTriangleMapSize);

		glm::ivec3 bi;
		for (bi.x = 0; bi.x < chunkResolution; bi.x++) {
			for (bi.y = 0; bi.y < chunkResolution; bi.y++) {
				for (bi.z = 0; bi.z < chunkResolution; bi.z++) {
					const std::array<Chunk::DensityType, 8> values = c.densityCubeAt(bi);

					const auto caseIndex = c.caseIndexFromVoxel(values);
					if (caseIndex == 255)
						continue; // solid block
					if (caseIndex == 0)
						continue; // air block

					const int numTriangles = case_to_numpolys[caseIndex];

					// for each triangle of the cube
					for (int t = 0; t < numTriangles; t++) {
						glm::ivec3 tri;

						// for each edge of the cube a triangle vertex is on
						for (int e = 0; e < 3; e++) {
							const int edgeIndex = edge_connect_list[caseIndex][t][e];

							const auto[value1, value2, vec1, vec2] = [&]() -> std::tuple<Chunk::DensityType, Chunk::DensityType, glm::ivec3, glm::ivec3> {
								switch (edgeIndex) {
									case 0 : return { values[0], values[1], bi + glm::ivec3(0, 0, 0), bi + glm::ivec3(0, 0, 1) };
									case 1 : return { values[1], values[2], bi + glm::ivec3(0, 0, 1), bi + glm::ivec3(1, 0, 1) };
									case 2 : return { values[2], values[3], bi + glm::ivec3(1, 0, 1), bi + glm::ivec3(1, 0, 0) };
									case 3 : return { values[3], values[0], bi + glm::ivec3(1, 0, 0), bi + glm::ivec3(0, 0, 0) };
									case 4 : return { values[4], values[5], bi + glm::ivec3(0, 1, 0), bi + glm::ivec3(0, 1, 1) };
									case 5 : return { values[5], values[6], bi + glm::ivec3(0, 1, 1), bi + glm::ivec3(1, 1, 1) };
									case 6 : return { values[6], values[7], bi + glm::ivec3(1, 1, 1), bi + glm::ivec3(1, 1, 0) };
									case 7 : return { values[7], values[4], bi + glm::ivec3(1, 1, 0), bi + glm::ivec3(0, 1, 0) };
									case 8 : return { values[0], values[4], bi + glm::ivec3(0, 0, 0), bi + glm::ivec3(0, 1, 0) };
									case 9 : return { values[1], values[5], bi + glm::ivec3(0, 0, 1), bi + glm::ivec3(0, 1, 1) };
									case 10: return { values[2], values[6], bi + glm::ivec3(1, 0, 1), bi + glm::ivec3(1, 1, 1) };
									case 11: return { values[3], values[7], bi + glm::ivec3(1, 0, 0), bi + glm::ivec3(1, 1, 0) };
									default: std::terminate();
								}
							}();

							const glm::vec3 vertex = interpolate(value1, value2, vec1, vec2);

							// lookup this vertex
							if (auto it = vertexMap.find(vertex); it != vertexMap.end())
								tri[e] = it->second;
							else {
								// calculate a new one
								RVertex v;
								v.position = c.toWorld(vertex);

								const glm::vec3 g1 = gradient(c, vec1);
								const glm::vec3 g2 = gradient(c, vec2);
								v.normal = -normalize(interpolate(value1, value2, g1, g2));

								tri[e] = (unsigned int)c.vertices.size();
								c.vertices.push_back(v);

								vertexMap[v.position] = tri[e];
							}
						}

						c.triangles.push_back(tri);
					}
				}
			}
		}
	}
}

auto ChunkCreator::getChunk(const glm::ivec3& chunkPos) -> Chunk {
	Chunk c(chunkPos);

	//noise::module::Perlin perlin;
	//perlin.SetOctaveCount(5);
	//perlin.SetFrequency(0.3);

	auto noise = [](glm::vec3 pos) {
		pos /= chunkResolution;

		// return (Chunk::DensityType)perlin.GetValue(pos.x, pos.y, pos.z);
		return stb_perlin_fbm_noise3(pos.x, pos.y, pos.z, 2.0f, 0.5f, global::noise::octaves);
		//return stb_perlin_turbulence_noise3(pos.x, pos.y, pos.z, 2.0f, 0.5f, global::noise::octaves, 0, 0, 0);
	};

	const unsigned int size = chunkResolution + 1 + 2; // + 1 for corners and + 2 for marging
	c.densities.resize(size * size * size);

	//Timer timer;

	for (unsigned int x = 0; x < size; x++) {
		for (unsigned int y = 0; y < size; y++) {
			for (unsigned int z = 0; z < size; z++) {
				glm::vec3 world = c.toWorld({x, y, z});
				c.densities[x * size * size + y * size + z] = noise(world);
			}
		}
	}

	//timer.tick();
	//cout << "Noise took " << timer.interval << " seconds" << endl;

	// create geometry using marching cubes
	marchChunk(c);

	//timer.tick();
	//cout << "Marching took " << timer.interval << " seconds" << endl;

	std::cout << "Created chunk:         " << chunkPos << '\n';

	//dumpTriangles("chunks/" + std::to_string(c.getId()) + "_triangles.ply", c.fullTriangles());
	//dumpLines("chunks/" + std::to_string(c.getId()) + "_aabb.ply", boxEdges(c.aabb()));

	return c;
}
