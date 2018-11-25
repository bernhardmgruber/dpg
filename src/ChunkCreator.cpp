//#include <noise/noise.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include <glm/glm.hpp>

#include <mutex>
#include <unordered_map>

#include "ChunkCreator.h"
#include "globals.h"

#include "tables.inc"

using namespace std;

const unsigned int CHUNK_TRIANGLE_MAP_INITIAL_SIZE = 3000;

auto ChunkCreator::getChunk(const glm::ivec3& chunkPos) -> Chunk {
	Chunk c(chunkPos);

	//noise::module::Perlin perlin;
	//perlin.SetOctaveCount(5);
	//perlin.SetFrequency(0.3);

	auto noise = [](glm::vec3 pos) {
		// return (Chunk::DensityType)perlin.GetValue(pos.x, pos.y, pos.z);
		return stb_perlin_fbm_noise3(pos.x, pos.y, pos.z, 2.0f, 0.5f, global::noise::octaves, 0, 0, 0);
		//return stb_perlin_turbulence_noise3(pos.x, pos.y, pos.z, 2.0f, 0.5f, global::noise::octaves, 0, 0, 0);
	};

	const unsigned int size = Chunk::RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging
	c.densities.resize(size * size * size);

	//Timer timer;

	for (unsigned int x = 0; x < size; x++) {
		for (unsigned int y = 0; y < size; y++) {
			for (unsigned int z = 0; z < size; z++) {
				glm::vec3 world = c.toWorld(x, y, z);
				c.densities[x * size * size + y * size + z] = noise(world);
			}
		}
	}

	//timer.tick();
	//cout << "Noise took " << timer.interval << " seconds" << endl;

	// create geometry using marching cubes
	marchChunk(&c);

	//timer.tick();
	//cout << "Marching took " << timer.interval << " seconds" << endl;

	cout << "Created chunk:         " << chunkPos << endl;

	return c;
}

void ChunkCreator::marchChunk(Chunk* c) {
	unordered_map<glm::vec3, unsigned int> vertexMap(CHUNK_TRIANGLE_MAP_INITIAL_SIZE);

	for (unsigned int x = 1; x < Chunk::RESOLUTION + 1; x++) {
		for (unsigned int y = 1; y < Chunk::RESOLUTION + 1; y++) {
			for (unsigned int z = 1; z < Chunk::RESOLUTION + 1; z++) {
				array<Chunk::DensityType, 8> values = c->voxelCubeAt(x, y, z);

				unsigned int caseIndex = c->caseIndexFromVoxel(values);

				if (caseIndex == 255)
					continue; // solid block
				if (caseIndex == 0)
					continue; // air block

				int numTriangles = case_to_numpolys[caseIndex];

				// for each triangle of the cube
				for (int t = 0; t < numTriangles; t++) {
					glm::ivec3 tri;

					// for each edge of the cube a triangle vertex is on
					for (int e = 0; e < 3; e++) {
						int edgeIndex = edge_connect_list[caseIndex][t][e];

						Chunk::DensityType value1;
						Chunk::DensityType value2;
						glm::ivec3 vec1;
						glm::ivec3 vec2;

						switch (edgeIndex) {
							case 0:
								value1 = values[0];
								value2 = values[1];
								vec1 = glm::ivec3(x, y, z);
								vec2 = glm::ivec3(x, y, z + 1);
								break;
							case 1:
								value1 = values[1];
								value2 = values[2];
								vec1 = glm::ivec3(x, y, z + 1);
								vec2 = glm::ivec3(x + 1, y, z + 1);
								break;
							case 2:
								value1 = values[2];
								value2 = values[3];
								vec1 = glm::ivec3(x + 1, y, z + 1);
								vec2 = glm::ivec3(x + 1, y, z);
								break;
							case 3:
								value1 = values[3];
								value2 = values[0];
								vec1 = glm::ivec3(x + 1, y, z);
								vec2 = glm::ivec3(x, y, z);
								break;

							case 4:
								value1 = values[4];
								value2 = values[5];
								vec1 = glm::ivec3(x, y + 1, z);
								vec2 = glm::ivec3(x, y + 1, z + 1);
								break;
							case 5:
								value1 = values[5];
								value2 = values[6];
								vec1 = glm::ivec3(x, y + 1, z + 1);
								vec2 = glm::ivec3(x + 1, y + 1, z + 1);
								break;
							case 6:
								value1 = values[6];
								value2 = values[7];
								vec1 = glm::ivec3(x + 1, y + 1, z + 1);
								vec2 = glm::ivec3(x + 1, y + 1, z);
								break;
							case 7:
								value1 = values[7];
								value2 = values[4];
								vec1 = glm::ivec3(x + 1, y + 1, z);
								vec2 = glm::ivec3(x, y + 1, z);
								break;

							case 8:
								value1 = values[0];
								value2 = values[4];
								vec1 = glm::ivec3(x, y, z);
								vec2 = glm::ivec3(x, y + 1, z);
								break;
							case 9:
								value1 = values[1];
								value2 = values[5];
								vec1 = glm::ivec3(x, y, z + 1);
								vec2 = glm::ivec3(x, y + 1, z + 1);
								break;
							case 10:
								value1 = values[2];
								value2 = values[6];
								vec1 = glm::ivec3(x + 1, y, z + 1);
								vec2 = glm::ivec3(x + 1, y + 1, z + 1);
								break;
							case 11:
								value1 = values[3];
								value2 = values[7];
								vec1 = glm::ivec3(x + 1, y, z);
								vec2 = glm::ivec3(x + 1, y + 1, z);
								break;

							default: cerr << "Invalid edge index: " << edgeIndex << endl; break;
						}

						glm::vec3 vertex = interpolate(value1, value2, vec1, vec2);

						// lookup this vertex
						auto it = vertexMap.find(vertex);
						if (it != vertexMap.end()) // we found it
							tri[e] = it->second;
						else {
							// calculate a new one
							Vertex v;
							v.position = c->toWorld(vertex);

							glm::vec3 normal1 = getNormal(c, vec1);
							glm::vec3 normal2 = getNormal(c, vec2);
							v.normal = normalize(interpolate(value1, value2, normal1, normal2));

							tri[e] = (unsigned int)c->vertices.size();
							c->vertices.push_back(v);

							vertexMap[v.position] = tri[e];
						}
					}

					c->triangles.push_back(tri);
				}
			}
		}
	}

	//cout << "Vertex map size " << vertexMap.size() << endl;
}

glm::vec3 ChunkCreator::getNormal(Chunk* c, const glm::uvec3& v) const {
	glm::vec3 grad;
	grad.x = c->voxelAt(v.x + 1, v.y, v.z) - c->voxelAt(v.x - 1, v.y, v.z);
	grad.y = c->voxelAt(v.x, v.y + 1, v.z) - c->voxelAt(v.x, v.y - 1, v.z);
	grad.z = c->voxelAt(v.x, v.y, v.z + 1) - c->voxelAt(v.x, v.y, v.z - 1);
	return normalize(grad);
}
