#pragma once

#include <filesystem>
#include <fstream>
#include <istream>
#include <iterator>
#include <vector>

namespace fs = std::filesystem;

template<typename T>
void read(std::istream& is, T& t) {
	is.read(reinterpret_cast<char*>(&t), sizeof(T));
}

template<typename T>
auto read(std::istream& is) {
	T t;
	is.read(reinterpret_cast<char*>(&t), sizeof(T));
	return t;
}

template<typename T>
void readVector(std::istream& is, std::vector<T>& v) {
	is.read(reinterpret_cast<char*>(v.data()), sizeof(T) * v.size());
}

template<typename T>
auto readVector(std::istream& is, std::size_t count) {
	std::vector<T> v(count);
	is.read(reinterpret_cast<char*>(v.data()), sizeof(T) * v.size());
	return v;
}

template<typename T>
void write(std::ostream& is, const T& t) {
	is.write(reinterpret_cast<const char*>(&t), sizeof(T));
}

template<typename T>
void writeVector(std::ostream& is, const std::vector<T>& v) {
	is.write(reinterpret_cast<const char*>(v.data()), sizeof(T) * v.size());
}

inline auto openFileIn(const fs::path& filename, int flags = 0) {
	std::ifstream file(filename, flags);
	if (!file)
		throw std::ios::failure("Failed to open file " + filename.string() + " for reading");
	return file;
}

inline void createDirectories(const fs::path& filename) {
	if (filename.has_parent_path())
		create_directories(filename.parent_path());
}

inline auto openFileOut(const fs::path& filename, int flags = 0) {
	createDirectories(filename);
	std::ofstream file(filename, flags);
	if (!file)
		throw std::ios::failure("Failed to open file " + filename.string() + " for writing");
	return file;
}

inline auto readTextFile(const fs::path& filename) {
	std::string content;
	auto file = openFileIn(filename);
	file.seekg(std::ios::end);
	content.reserve(file.tellg());
	file.seekg(std::ios::beg);
	content.assign(std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{});
	return content;
}
