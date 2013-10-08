#pragma once

#include <cmath>
#include <iostream>
#include <stdexcept>

#define EPSILON 0.0001
#define PI 3.14159265

template<typename T>
struct Vector2
{
    T x, y;

    Vector2()
    {};

    Vector2(T x, T y)
        : x(x), y(y)
    {};

	template<typename U>
	Vector2(Vector2<U> v)
		: x(v.x), y(v.y)
	{};

	T& operator[](int index)
	{
		switch(index)
		{
		case 0: return x;
		case 1: return y;
		default: throw std::out_of_range("invalid index");
		}
	}
};

typedef Vector2<float> Vector2F;
typedef Vector2<int> Vector2I;

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

typedef Vector3<float> Vector3F;
typedef Vector3<int> Vector3I;
typedef Vector3<unsigned int> Vector3UI;

struct Vertex
{
    Vector3F position;
    Vector3F normal;
};

struct Triangle
{
	Vertex vertices[3];
};

std::ostream& operator<<(std::ostream& os, Vector3F v);
std::ostream& operator<<(std::ostream& os, Vector3I v);
std::ostream& operator<<(std::ostream& os, Vector2F v);

Vector3F operator-(Vector3F v1, Vector3F v2);                   // Vector substraction
Vector3F operator+(Vector3F v1, Vector3F v2);                   // Vector addition
Vector3I operator+(Vector3I v1, Vector3I v2);                   // Vector addition
Vector3F operator*(float f, Vector3F v);                        // Vector multiplication with a scalar
Vector3F operator*(Vector3F v, float f);                        // Vector multiplication with a scalar
Vector3F operator/(Vector3F v, float f);                        // Vector division through a scalar
bool operator==(Vector3F v1, Vector3F v2);
bool operator!=(Vector3F v1, Vector3F v2);

float length(Vector3F v);                                       // Returns the length of a vector
Vector3F normalize(Vector3F v);                                 // Returns the normalized vector
float dotProduct(Vector3F v1, Vector3F v2);                     // Returns the dot product of the two vectors given
Vector3F crossProduct(Vector3F v1, Vector3F v2);                // Returns the cross product vector of the two vectors given

bool pointInBox(Vector3F vPoint, short vMin[3], short vMax[3]); // Returns a bool spezifing whether or not a point is in the defined box
bool pointInPlane(Vector3F vPoint, Vector3F vNormal, float fDist);

template<typename T>
float distance(Vector3<T> a, Vector3<T> b)
{
	return length(a - b);
}

Vector3F rotateX(float a, Vector3F v);
Vector3F rotateY(float a, Vector3F v);
Vector3F rotateZ(float a, Vector3F v);

template<typename T>
T degToRad(T deg)
{
	return (deg / (T)180.0) * (T)PI;
}

template<typename T>
T radToDeg(T rad)
{
	return (rad / (T)PI) * (T)180.0;
}
