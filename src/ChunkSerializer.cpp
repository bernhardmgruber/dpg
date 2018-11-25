#include "globals.h"
#include "utils.h"
#include <fstream>
#include <string>
#include <utility>

#include "ChunkSerializer.h"

ChunkSerializer::ChunkSerializer(std::filesystem::path chunkDir)
	: m_chunkDir(std::move(chunkDir)) {

	if (!exists(chunkDir))
		return;
	for (auto& e : std::filesystem::directory_iterator{m_chunkDir}) {
		const auto filename = e.path().string();

		static_assert(is_same<uint64_t, Chunk::IdType>::value, "Chunk::IdType is assumed to be uint64_t");
		char* p = nullptr;
		Chunk::IdType id = std::strtoll(filename.c_str(), &p, 16);
		if (*p != filename.size()) {
			cout << "Warning: " << filename << " in chunk cache" << endl;
			continue;
		}
		availableChunks.insert(id);
	}
}

ChunkSerializer::~ChunkSerializer() = default;

bool ChunkSerializer::hasChunk(const glm::ivec3& chunkPos) {
	return availableChunks.find(Chunk::ChunkGridCoordinateToId(chunkPos)) != availableChunks.end();
}

void ChunkSerializer::storeChunk(const Chunk* chunk) {
	if (!global::enableChunkCache)
		return;

	const unsigned int size = Chunk::RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging

	// write chunk to disk
	ofstream file(m_chunkDir.string() + toHexString(chunk->getId()), ios::binary);
	file.write(reinterpret_cast<char*>(chunk->densities), size * size * size * sizeof(Chunk::DensityType));
	file << static_cast<size_t>(chunk->vertices.size());
	file.write((char*)chunk->vertices.data(), chunk->vertices.size() * sizeof(Vertex));
	file << static_cast<size_t>(chunk->triangles.size());
	file.write((char*)chunk->triangles.data(), chunk->triangles.size() * sizeof(glm::uvec3));
	file.close();

	cout << "Wrote chunk from disk: " << chunk->getChunkGridPositon() << endl;
}

Chunk* ChunkSerializer::getChunk(const glm::ivec3& chunkPos) {
	Chunk::IdType chunkId = Chunk::ChunkGridCoordinateToId(chunkPos);

	if (!global::enableChunkCache || availableChunks.find(chunkId) == availableChunks.end())
		return nullptr; // this chunk is not available in the chunk directory

	// read chunk from disk
	auto* c = new Chunk(chunkId);
	const unsigned int size = Chunk::RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging

	ifstream file(m_chunkDir.string() + toHexString(chunkId), ios::binary);
	if (!file)
		throw runtime_error("could not open chunk file " + m_chunkDir.string() + to_string(chunkId));
	c->densities = new Chunk::DensityType[size * size * size];
	file.read(reinterpret_cast<char*>(c->densities), size * size * size * sizeof(Chunk::DensityType));
	size_t verticesCount = 0;
	file >> verticesCount;
	if (verticesCount > 0) {
		c->vertices.resize(verticesCount);
		file.read(reinterpret_cast<char*>(c->vertices.data()), verticesCount * sizeof(Vertex));
	}
	size_t trianglesCount = 0;
	file >> trianglesCount;
	if (trianglesCount > 0) {
		c->triangles.resize(trianglesCount);
		file.read(reinterpret_cast<char*>(c->triangles.data()), trianglesCount * sizeof(Vertex));
	}
	file.close();

	cout << "Read chunk from disk:  " << chunkPos << endl;

	return c;
}
