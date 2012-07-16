#ifndef MATHLIB_H
#define MATHLIB_H

#include <cmath>
#include <iostream>

#define EPSILON 0.0001
#define PI 3.14159265

#define DEGTORAD(x) ((x) * PI / 180.0)
#define RADTODEG(x) ((x) * 180.0 / PI)

// structure for a float double
struct Vector2D
{
    float x, y;

    Vector2D()
    {};

    Vector2D(float x, float y)
        : x(x), y(y)
    {};
};

// structure for a float triple
struct Vector3D
{
    float x, y, z;

    Vector3D()
    {};

    Vector3D(float x, float y, float z)
        : x(x), y(y), z(z)
    {};
};

struct Triangle
{
    Vector3D vertices[3];
};

std::ostream& operator<<(std::ostream& os, Vector3D v);
std::ostream& operator<<(std::ostream& os, Vector2D v);

Vector3D operator-(Vector3D v1, Vector3D v2);                   // Vector substraction
Vector3D operator+(Vector3D v1, Vector3D v2);                   // Vector addition
Vector3D operator*(float f, Vector3D v);                        // Vector multiplication with a scalar
Vector3D operator*(Vector3D v, float f);                        // Vector multiplication with a scalar
Vector3D operator/(Vector3D v, float f);                        // Vector division through a scalar
bool operator==(Vector3D v1, Vector3D v2);
bool operator!=(Vector3D v1, Vector3D v2);
float Length(Vector3D v);                                       // Returns the length of a vector
Vector3D Normalize(Vector3D v);                                 // Returns the normalized vector
float DotProduct(Vector3D v1, Vector3D v2);                     // Returns the dot product of the two vectors given
Vector3D CrossProduct(Vector3D v1, Vector3D v2);                // Returns the cross product vector of the two vectors given
bool PointInBox(Vector3D vPoint, short vMin[3], short vMax[3]); // Returns a bool spezifing whether or not a point is in the defined box
bool PointInPlane(Vector3D vPoint, Vector3D vNormal, float fDist);

Vector3D RotateX(float a, Vector3D v);
Vector3D RotateY(float a, Vector3D v);
Vector3D RotateZ(float a, Vector3D v);

#endif
