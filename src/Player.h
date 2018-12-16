#pragma once

#include <glm/vec3.hpp>
#include <cstdint>

class World;
class Camera;

class Player {
public:
	void update(double t, float xDelta, float yDelta, uint8_t directions, const World& world, Camera& camera);

	glm::vec3 velocity;
	bool onGround = false;
};
