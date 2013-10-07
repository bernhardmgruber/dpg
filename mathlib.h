#pragma once

#include <cmath>
#include <iostream>

#define EPSILON 0.0001
#define PI 3.14159265

#define DEGTORAD(x) ((x) * PI / 180.0)
#define RADTODEG(x) ((x) * 180.0 / PI)

template<typename T>
struct Vector2
{
    T x, y;

    Vector2()
    {};

    Vector2(T x, T y)
        : x(x), y(y)
    {};
};

typedef Vector2<float> Vector2F;

template<typename T>
struct Vector3
{
    T x, y, z;

    Vector3()
    {};

    Vector3(T x, T y, T z)
        : x(x), y(y), z(z)
    {};
};

typedef Vector3<float> Vector3F;
typedef Vector3<int> Vector3I;

struct Triangle
{
    Vector3F vertices[3];
    Vector3F normals[3];
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
float Length(Vector3F v);                                       // Returns the length of a vector
Vector3F Normalize(Vector3F v);                                 // Returns the normalized vector
float DotProduct(Vector3F v1, Vector3F v2);                     // Returns the dot product of the two vectors given
Vector3F CrossProduct(Vector3F v1, Vector3F v2);                // Returns the cross product vector of the two vectors given
bool PointInBox(Vector3F vPoint, short vMin[3], short vMax[3]); // Returns a bool spezifing whether or not a point is in the defined box
bool PointInPlane(Vector3F vPoint, Vector3F vNormal, float fDist);

Vector3F RotateX(float a, Vector3F v);
Vector3F RotateY(float a, Vector3F v);
Vector3F RotateZ(float a, Vector3F v);
