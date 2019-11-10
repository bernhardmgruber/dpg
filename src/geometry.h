#pragma once

#include <glm/glm.hpp>

#include <array>
#include <optional>
#include <vector>

#include "IO.h"

class Line {
public:
	Line() = default;
	Line(glm::vec3 a, glm::vec3 b)
		: values{a, b} {}

	auto operator[](std::size_t i) -> glm::vec3& {
		return values[i];
	}

	auto operator[](std::size_t i) const -> const glm::vec3& {
		return values[i];
	}

	auto begin() { return values.begin(); }
	auto begin() const { return values.begin(); }
	auto end() { return values.end(); }
	auto end() const { return values.end(); }

private:
	std::array<glm::vec3, 2> values;
};

class Triangle {
public:
	Triangle() = default;
	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
		: values{a, b, c} {}

	auto operator[](std::size_t i) -> glm::vec3& {
		return values[i];
	}

	auto operator[](std::size_t i) const -> const glm::vec3& {
		return values[i];
	}

	auto begin() { return values.begin(); }
	auto begin() const { return values.begin(); }
	auto end() { return values.end(); }
	auto end() const { return values.end(); }

	auto normal() const -> glm::vec3 {
		return glm::normalize(glm::cross(values[1] - values[0], values[1] - values[2]));
	}

private:
	std::array<glm::vec3, 3> values;
};

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

struct BoundingBox {
	glm::vec3 lower;
	glm::vec3 upper;
};

template<typename Range>
void dumpTriangles(const std::filesystem::path& path, const Range& triangles) {
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
	for (const Triangle& t : triangles)
		write(f, t);

	const uint8_t vertexCount = 3;
	for (uint32_t i = 0; i < static_cast<uint32_t>(triangles.size()); i++) {
		write(f, vertexCount);
		write(f, (i * 3) + 0);
		write(f, (i * 3) + 1);
		write(f, (i * 3) + 2);
	}
	f.close();
}

template<typename Range>
void dumpLines(const std::filesystem::path& path, const Range& lines) {
	std::vector<Triangle> triangles;
	triangles.reserve(lines.size());
	for (const Line& l : lines)
		triangles.emplace_back(l[0], l[0], l[1]);
	dumpTriangles(path, triangles);
}

template<typename Range>
void dumpPoints(const std::filesystem::path& path, const Range& points) {
	auto f = openFileOut(path, std::ios::binary);
	f
		<< "ply\n"
		<< "format binary_little_endian 1.0\n"
		<< "element vertex " << points.size() << "\n"
		<< "property float x\n"
		<< "property float y\n"
		<< "property float z\n"
		<< "end_header\n";
	for (const glm::vec3& p : points)
		write(f, p);
	f.close();
}

constexpr auto triangleInsideEpsilon = 1.f;

inline auto intersectForDistance(Ray ray, Triangle triangle) -> std::optional<float> {
	const float EPSILON = 1e-7f;
	const glm::vec3 vertex0 = triangle[0];
	const glm::vec3 edge1 = triangle[1] - vertex0;
	const glm::vec3 edge2 = triangle[2] - vertex0;
	const glm::vec3 h = cross(ray.direction, edge2);
	const float a = dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return {}; // ray is parallel to triangle
	const float f = 1.0f / a;
	const glm::vec3 s = ray.origin - vertex0;
	const float u = f * dot(s, h);
	if (u < 0 - triangleInsideEpsilon || u > 1 + triangleInsideEpsilon)
		return {};
	const glm::vec3 q = cross(s, edge1);
	const float v = f * dot(ray.direction, q);
	if (v < 0 - triangleInsideEpsilon || v > 1 + triangleInsideEpsilon || u + v > 1 + triangleInsideEpsilon)
		return {};
	return f * dot(edge2, q);
}

inline auto intersectForDistance(Ray ray, const std::vector<Triangle>& triangles) -> std::optional<float> {
	float minD = std::numeric_limits<float>::max();
	for (const auto& t : triangles)
		if (const auto d = intersectForDistance(ray, t))
			if (d && *d < minD)
				minD = *d;
	if (minD == std::numeric_limits<float>::max())
		return {};
	return minD;
}

inline auto intersect(Ray ray, Triangle triangle) -> std::optional<glm::vec3> {
	const auto t = intersectForDistance(ray, triangle);
	if (!t || *t < 0)
		return {};

	//static auto i = 0;
	//i++;
	//dump("intersect/" + std::to_string(i) + "_triangles.ply", {triangle});
	//dump("intersect/" + std::to_string(i) + "_ray.ply", {ray.origin, ray.origin + ray.direction * 100.0f});

	return ray.origin + ray.direction * *t;
}

inline auto intersect(Ray ray, const std::vector<Triangle>& triangles) -> std::optional<glm::vec3> {
	const auto t = intersectForDistance(ray, triangles);
	if (!t || *t < 0)
		return {};

	//static auto i = 0;
	//i++;
	//dump("intersectV/" + std::to_string(i) + "_triangles.ply", triangles);
	//dump("intersectV/" + std::to_string(i) + "_ray.ply", {Triangle{ray.origin, ray.origin, ray.origin + ray.direction * 100.0f}});

	return ray.origin + ray.direction * *t;
}

// from: https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
// with nan check from: https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/
inline auto intersectBox(BoundingBox box, Ray ray) -> std::optional<std::pair<float, float>> {
	const auto dirfrac = 1.0f / ray.direction;
	const auto t0 = (box.lower - ray.origin) * dirfrac;
	const auto t1 = (box.upper - ray.origin) * dirfrac;

	auto tmin = std::min(t0[0], t1[0]);
	auto tmax = std::max(t0[0], t1[0]);
	for (auto i : {1, 2}) {
		tmin = std::max(tmin, std::min(std::min(t0[i], t1[i]), tmax));
		tmax = std::min(tmax, std::max(std::max(t0[i], t1[i]), tmin));
	}

	if (tmax > std::max(tmin, 0.0f))
		return std::pair{tmin, tmax};
	return {};
}

inline auto clamp(glm::vec3 p, BoundingBox box) {
	for (auto i = 0; i < 3; i++)
		p[i] = std::clamp(p[i], box.lower[i], box.upper[i]);
	return p;
}

inline auto boxVertices(BoundingBox box) -> std::array<glm::vec3, 8> {
	const auto& l = box.lower;
	const auto& u = box.upper;

	const auto vertices = std::array<glm::vec3, 8>{
		glm::vec3{l[0], l[1], l[2]},
		glm::vec3{l[0], l[1], u[2]},
		glm::vec3{l[0], u[1], l[2]},
		glm::vec3{l[0], u[1], u[2]},
		glm::vec3{u[0], l[1], l[2]},
		glm::vec3{u[0], l[1], u[2]},
		glm::vec3{u[0], u[1], l[2]},
		glm::vec3{u[0], u[1], u[2]}};

	return vertices;
}

inline auto boxTriangles(BoundingBox box) -> std::array<Triangle, 12> {
	const auto v = boxVertices(box);

	const auto triangles = std::array<Triangle, 12>{
		Triangle{v[2], v[6], v[0]},
		Triangle{v[0], v[6], v[4]},
		Triangle{v[6], v[5], v[4]},
		Triangle{v[4], v[5], v[0]},

		Triangle{v[5], v[1], v[0]},
		Triangle{v[0], v[1], v[2]},
		Triangle{v[1], v[3], v[2]},
		Triangle{v[2], v[3], v[6]},

		Triangle{v[3], v[7], v[6]},
		Triangle{v[6], v[7], v[5]},
		Triangle{v[7], v[3], v[5]},
		Triangle{v[5], v[3], v[1]}};

	return triangles;
}

inline auto boxEdges(BoundingBox box) -> std::array<Line, 12> {
	const auto v = boxVertices(box);

	const auto triangles = std::array<Line, 12>{
		Line{v[0], v[1]},
		Line{v[0], v[2]},
		Line{v[1], v[3]},
		Line{v[2], v[3]},

		Line{v[4], v[5]},
		Line{v[4], v[6]},
		Line{v[5], v[7]},
		Line{v[6], v[7]},

		Line{v[0], v[4]},
		Line{v[1], v[5]},
		Line{v[2], v[6]},
		Line{v[3], v[7]}};

	return triangles;
}
