#include "geometry.h"

void dump(const std::filesystem::path & path, const std::vector<Triangle>& triangles) {
	auto f = openFileOut(path, std::ios::binary);
	f
		<< "ply\n"
		<< "format binary_little_endian 1.0\n"
		<< "element vertex " << triangles.size() * 3 << "\n"
		<< "property float x\n"
		<< "property float y\n"
		<< "property float z\n"
		<< "element face " << triangles.size() << "\n"
		<< "property list uchar uint vertex_indices\n"
		<< "end_header\n";
	writeVector(f, triangles);
	
	const uint8_t vertexCount = 3;
	for (uint32_t i = 0; i < static_cast<uint32_t>(triangles.size()); i++) {
		write(f, vertexCount);
		write(f, (i * 3) + 0);
		write(f, (i * 3) + 1);
		write(f, (i * 3) + 2);
	}
	f.close();
}

void dump(const std::filesystem::path& path, const std::vector<Line>& lines) {
	std::vector<Triangle> triangles(lines.size());
	for (const auto& l : lines)
		triangles.emplace_back(l[0], l[0], l[1]);
	dump(path, triangles);
}

void dump(const std::filesystem::path& path, const std::vector<glm::vec3>& points) {
	auto f = openFileOut(path, std::ios::binary);
	f
		<< "ply\n"
		<< "format binary_little_endian 1.0\n"
		<< "element vertex " << points.size() << "\n"
		<< "property float x\n"
		<< "property float y\n"
		<< "property float z\n"
		<< "end_header\n";
	writeVector(f, points);
	f.close();
}
