#pragma once

#include <glm/glm.hpp>

#include <array>
#include <optional>
#include <vector>

#include "IO.h"

class Triangle {
public:
	Triangle() = default;
	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
		: values{ {a, b, c} } {}

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
	std::array<glm::vec3, 3> values;
};

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

void dump(const std::filesystem::path& path, const std::vector<Triangle>& triangles);
void dump(const std::filesystem::path& path, const std::vector<glm::vec3>& points);

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
	if (u < 0 || u > 1)
		return {};
	const glm::vec3 q = cross(s, edge1);
	const float v = f * dot(ray.direction, q);
	if (v < 0 || u + v > 1)
		return {};
	return f * dot(edge2, q);
}

inline auto intersect(Ray ray, Triangle triangle) -> std::optional<glm::vec3> {
	const auto t = intersectForDistance(ray, triangle);
	if (!t || *t < 0)
		return {};

	static auto i = 0;
	i++;
	dump("intersect/" + std::to_string(i) + "_triangles.ply", { triangle });
	dump("intersect/" + std::to_string(i) + "_ray.ply", { ray.origin, ray.origin + ray.direction * 100.0f });

	return ray.origin + ray.direction * *t;
}

inline auto intersect(Ray ray, const std::vector<Triangle>& triangles) -> std::optional<glm::vec3> {
	float minD = std::numeric_limits<float>::max();
	for (const auto& t : triangles)
		if (const auto d = intersectForDistance(ray, t))
			if (d && *d < minD)
				minD = *d;
	if (minD == std::numeric_limits<float>::max())
		return {};

	static auto i = 0;
	i++;
	dump("intersectV/" + std::to_string(i) + "_triangles.ply", triangles);
	dump("intersectV/" + std::to_string(i) + "_ray.ply", { Triangle{ ray.origin, ray.origin, ray.origin + ray.direction * 100.0f } });

	return ray.origin + ray.direction * minD;
}