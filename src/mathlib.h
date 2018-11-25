#pragma once

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "mathtypes.h"

#define EPSILON 0.0001
#define PI 3.14159265

std::ostream& operator<<(std::ostream& os, glm::vec3 v);
std::ostream& operator<<(std::ostream& os, glm::ivec3 v);
std::ostream& operator<<(std::ostream& os, glm::vec2 v);

bool pointInBox(glm::vec3 vPoint, const short vMin[3], const short vMax[3]); // Returns a bool spezifing whether or not a point is in the defined box
bool pointInPlane(glm::vec3 vPoint, glm::vec3 vNormal, float fDist);

template<typename T>
T degToRad(T deg) {
	return (deg / (T)180.0) * (T)PI;
}

template<typename T>
T radToDeg(T rad) {
	return (rad / (T)PI) * (T)180.0;
}
