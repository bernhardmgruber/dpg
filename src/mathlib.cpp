#include <cmath>
#include <iomanip>

#include "mathlib.h"

using namespace std;

ostream& operator<<(ostream& os, glm::vec3 v) {
	os << "(" << setw(2) << v.x << ", " << v.y << ", " << v.z << ")";

	return os;
}

ostream& operator<<(ostream& os, glm::ivec3 v) {
	os << "(" << setw(2) << v.x << ", " << v.y << ", " << v.z << ")";

	return os;
}

ostream& operator<<(ostream& os, glm::vec2 v) {
	os << "(" << setw(2) << v.x << ", " << v.y << ")";

	return os;
}

glm::vec3 operator-(glm::vec3 v1, glm::vec3 v2) {
	glm::vec3 resultVector;
	resultVector.x = v1.x - v2.x;
	resultVector.y = v1.y - v2.y;
	resultVector.z = v1.z - v2.z;
	return resultVector;
}

glm::vec3 operator+(glm::vec3 v1, glm::vec3 v2) {
	glm::vec3 resultVector;
	resultVector.x = v1.x + v2.x;
	resultVector.y = v1.y + v2.y;
	resultVector.z = v1.z + v2.z;
	return resultVector;
}

glm::ivec3 operator+(glm::ivec3 v1, glm::ivec3 v2) {
	glm::ivec3 resultVector;
	resultVector.x = v1.x + v2.x;
	resultVector.y = v1.y + v2.y;
	resultVector.z = v1.z + v2.z;
	return resultVector;
}

glm::vec3 operator*(float f, glm::vec3 v) {
	v.x *= f;
	v.y *= f;
	v.z *= f;
	return v;
}

glm::vec3 operator*(glm::vec3 v, float f) {
	v.x *= f;
	v.y *= f;
	v.z *= f;
	return v;
}

glm::vec3 operator/(glm::vec3 v, float f) {
	v.x /= f;
	v.y /= f;
	v.z /= f;
	return v;
}

bool operator==(glm::vec3 v1, glm::vec3 v2) {
	glm::vec3 v = v1 - v2;

	if (v.x != 0)
		return false;
	if (v.y != 0)
		return false;
	if (v.z != 0)
		return false;

	return true;
}

bool operator!=(glm::vec3 v1, glm::vec3 v2) {
	return !(v1 == v2);
}

float length(glm::vec3 v) {
	return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

glm::vec3 normalize(glm::vec3 v) {
	return v / length(v);
}

float dotProduct(glm::vec3 v1, glm::vec3 v2) {
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

glm::vec3 crossProduct(glm::vec3 v1, glm::vec3 v2) {
	glm::vec3 perpendicularVector;
	perpendicularVector.x = (v2.y * v1.z) - (v2.z * v1.y);
	perpendicularVector.y = (v2.z * v1.x) - (v2.x * v1.z);
	perpendicularVector.z = (v2.x * v1.y) - (v2.y * v1.x);
	return perpendicularVector;
}

bool pointInBox(glm::vec3 vPoint, const short vMin[3], const short vMax[3]) {
	return ((float)vMin[0] <= vPoint.x && vPoint.x <= (float)vMax[0] &&
			(float)vMin[1] <= vPoint.y && vPoint.y <= (float)vMax[1] &&
			(float)vMin[2] <= vPoint.z && vPoint.z <= (float)vMax[2]) ||
		((float)vMin[0] >= vPoint.x && vPoint.x >= (float)vMax[0] &&
			(float)vMin[1] >= vPoint.y && vPoint.y >= (float)vMax[1] &&
			(float)vMin[2] >= vPoint.z && vPoint.z >= (float)vMax[2]);
}

bool pointInPlane(glm::vec3 vPoint, glm::vec3 vNormal, float fDist) {
	return fabs(dotProduct(vPoint, vNormal) - fDist) < EPSILON;
}

glm::vec3 rotateX(float a, glm::vec3 v) {
	a = degToRad(a);

	glm::vec3 res;
	res.x = v.x;
	res.y = v.y * cos(a) + v.z * -sin(a);
	res.z = v.y * sin(a) + v.z * cos(a);
	return res;
}

glm::vec3 rotateY(float a, glm::vec3 v) {
	a = degToRad(a);

	glm::vec3 res;
	res.x = v.x * cos(a) + v.z * sin(a);
	res.y = v.y;
	res.z = v.x * -sin(a) + v.z * cos(a);
	return res;
}

glm::vec3 rotateZ(float a, glm::vec3 v) {
	a = degToRad(a);

	glm::vec3 res;
	res.x = v.x * cos(a) + v.y * -sin(a);
	res.y = v.x * sin(a) + v.y * cos(a);
	res.z = v.z;
	return res;
}
