#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using Vector2F = glm::vec2;
using Vector2I = glm::ivec2;

template<typename T>
struct Vector3
{
    T x, y, z;

    Vector3()
    {};

    Vector3(T x, T y, T z)
        : x(x), y(y), z(z)
    {};

    template<typename U>
    Vector3(Vector3<U> v)
        : x((T)v.x), y((T)v.y), z((T)v.z)
    {};

    T& operator[](int index)
    {
        switch(index)
        {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw std::out_of_range("invalid index");
        }
    }
};

using Vector3F = glm::vec3;
using Vector3I = glm::ivec3;
using Vector3UI = glm::uvec3;

struct Vertex
{
    Vector3F position;
    Vector3F normal;
};

struct Triangle
{
    Vertex vertices[3];
};

struct BoundingBox
{
    Vector3F lower;
    Vector3F upper;

    BoundingBox(const Vector3F& lower, const Vector3F& upper)
        : lower(lower), upper(upper) { };
};

//
// hashes
//

namespace std
{
    template<>
    struct hash<Vector3UI>
    {
        size_t operator() (const Vector3UI& v) const { return v.x ^ (v.y << 11) ^ (v.z << 22); }
    };

    template<>
    struct hash<Vector3I>
    {
        size_t operator() (const Vector3I& v) const { return v.x ^ (v.y << 11) ^ (v.z << 22); }
    };

    template<>
    struct hash<Vector3F>
    {
        size_t operator() (const Vector3F& v) const { return (int)v.x ^ ((int)v.y << 11) ^ ((int)v.z << 22); }
    };
}