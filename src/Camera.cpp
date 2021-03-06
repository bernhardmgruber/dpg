#include "Camera.h"

#include <algorithm>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>

auto Camera::viewVector() const -> glm::vec3 {
	glm::vec3 v;
	v.x = 1;
	v.y = 0;
	v.z = 0;

	// rotate pitch along -y
	v = glm::rotateY(v, degToRad(-pitch));

	// rotate yaw along z
	v = glm::rotateZ(v, degToRad(yaw));

	return v;
}

void Camera::update(double t, float xDelta, float yDelta, uint8_t directions) {
	yaw += lookSensitivity * xDelta;
	yaw = std::fmod(yaw, 360.0f);

	pitch += lookSensitivity * yDelta;
	pitch = std::clamp(pitch, -90.0f, 90.0f);

	double fTmpMoveSens = moveSensitivity * t;

	if (directions & Up)
		position.z += fTmpMoveSens;

	if (directions & Down)
		position.z -= fTmpMoveSens;

	// TODO(bernh): If strafing and moving reduce speed to keep total move per frame constant
	const auto cosYaw = std::cos(degToRad(yaw)) * fTmpMoveSens;
	const auto sinYaw = std::sin(degToRad(yaw)) * fTmpMoveSens;

	if (directions & Forward) {
		position.x += cosYaw;
		position.y += sinYaw;
	}

	if (directions & Backward) {
		position.x -= cosYaw;
		position.y -= sinYaw;
	}

	if (directions & Left) {
		position.x -= sinYaw;
		position.y += cosYaw;
	}

	if (directions & Right) {
		position.x += sinYaw;
		position.y -= cosYaw;
	}
}

auto Camera::viewMatrix() const -> glm::mat4 {
	// in BSP v30 the z axis points up and we start looking parallel to x axis
	glm::mat4 mat = glm::eulerAngleXZ(degToRad(-pitch - 90.0f), degToRad(-yaw + 90.0f));
	mat = glm::translate(mat, -position); // move
	return mat;
}
