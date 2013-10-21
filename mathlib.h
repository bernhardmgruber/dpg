#pragma once

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "mathtypes.h"

#define EPSILON 0.0001
#define PI 3.14159265

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
