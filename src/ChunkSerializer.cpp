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

		static_assert(is_same<uint64_t, IdType>::value, "Chunk::IdType is assumed to be uint64_t");
		char* p = nullptr;
		IdType id = std::strtoll(filename.c_str(), &p, 16);
		if (*p != filename.size()) {
			cout << "Warning: " << filename << " in chunk cache" << endl;
			continue;
		}
		availableChunks.insert(id);
	}
}

ChunkSerializer::~ChunkSerializer() = default;

bool ChunkSerializer::hasChunk(const glm::ivec3& chunkPos) {
	return availableChunks.find(ChunkGridCoordinateToId(chunkPos)) != availableChunks.end();
}

void ChunkSerializer::storeChunk(const Chunk& chunk) {
	if (!global::enableChunkCache)
		return;

	const unsigned int size = chunkResolution + 1 + 2; // + 1 for corners and + 2 for marging

	// write chunk to disk
	create_directory(m_chunkDir);
	std::ofstream file(m_chunkDir / toHexString(chunk.getId()), ios::binary);
	file.write(reinterpret_cast<const char*>(chunk.densities.data()), size * size * size * sizeof(Chunk::DensityType));
	file << static_cast<size_t>(chunk.vertices.size());
	file.write((char*)chunk.vertices.data(), chunk.vertices.size() * sizeof(RVertex));
	file << static_cast<size_t>(chunk.triangles.size());
	file.write((char*)chunk.triangles.data(), chunk.triangles.size() * sizeof(glm::uvec3));
	file.close();

	cout << "Wrote chunk from disk: " << chunk.getChunkGridPositon() << endl;
}

auto ChunkSerializer::getChunk(const glm::ivec3& chunkPos) -> Chunk {
	IdType chunkId = ChunkGridCoordinateToId(chunkPos);

	if (availableChunks.find(chunkId) == availableChunks.end())
		throw std::runtime_error("chunk requested from serializer, but not available");

	// read chunk from disk
	Chunk c(chunkId);
	const unsigned int size = chunkResolution + 1 + 2; // + 1 for corners and + 2 for marging

	const auto chunkFile = m_chunkDir / toHexString(chunkId);
	std::ifstream file(chunkFile, ios::binary);
	if (!file)
		throw runtime_error("could not open chunk file " + chunkFile.string());
	c.densities.resize(size * size * size);
	file.read(reinterpret_cast<char*>(c.densities.data()), size * size * size * sizeof(Chunk::DensityType));
	size_t verticesCount = 0;
	file >> verticesCount;
	if (verticesCount > 0) {
		c.vertices.resize(verticesCount);
		file.read(reinterpret_cast<char*>(c.vertices.data()), verticesCount * sizeof(RVertex));
	}
	size_t trianglesCount = 0;
	file >> trianglesCount;
	if (trianglesCount > 0) {
		c.triangles.resize(trianglesCount);
		file.read(reinterpret_cast<char*>(c.triangles.data()), trianglesCount * sizeof(RVertex));
	}
	file.close();

	cout << "Read chunk from disk:  " << chunkPos << endl;

	return c;
}
