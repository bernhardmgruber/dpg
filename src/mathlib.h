#pragma once

#include <boost/math/constants/constants.hpp>

#include <cmath>
#include <iostream>

#include "mathtypes.h"

std::ostream& operator<<(std::ostream& os, glm::vec3 v);
std::ostream& operator<<(std::ostream& os, glm::ivec3 v);
std::ostream& operator<<(std::ostream& os, glm::vec2 v);

bool pointInBox(glm::vec3 vPoint, const short vMin[3], const short vMax[3]); // Returns a bool spezifing whether or not a point is in the defined box
bool pointInPlane(glm::vec3 vPoint, glm::vec3 vNormal, float fDist);

template<typename T>
T degToRad(T deg) {
	return (deg / (T)180.0) * boost::math::constants::pi<T>();
}

template<typename T>
T radToDeg(T rad) {
	return (rad / boost::math::constants::pi<T>()) * (T)180.0;
}

template <typename T>
inline T interpolate(float da, float db, T va, T vb) {
	const float part = fabs(da) / (fabs(da) + fabs(db));
	return va + (vb - va) * part;
}
