#include "Player.h"

#include "Camera.h"
#include "World.h"

void Player::update(double t, float xDelta, float yDelta, uint8_t directions, const World& world, Camera& camera) {
	const auto oldPos = camera.position;
	camera.update(t, xDelta, yDelta, directions);
	const auto newPos = camera.position;

	// we place the camera at the first collision on the line from old to new
	camera.position = world.trace(oldPos, newPos);
}
