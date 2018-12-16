#include "Player.h"

#include "Camera.h"
#include "World.h"

namespace {
	constexpr auto gravity = 7.0f;
	constexpr auto jumpStrength = 9.0f;
}

void Player::update(double t, float xDelta, float yDelta, uint8_t directions, const World& world, Camera& camera) {
	const auto up = directions & Up;
	directions &= ~(Up | Down); // clear up and down, those are done by jump and fall mechanics

	if (onGround && up)
		velocity.z += jumpStrength;

	const auto oldPos = camera.position;
	camera.update(t, xDelta, yDelta, directions);
	auto newPos = camera.position;

	velocity.z -= gravity * t;

	newPos += velocity * (float)t;

	// we place the camera at the first collision on the line from old to new
	const auto result = world.trace(oldPos, newPos);
	camera.position = result.end;
	if (result.collision)
		velocity = {};
	onGround = result.collision && (result.end - oldPos).z <= 0;

	//static auto counter = 0;
	//dumpLines("player/move" + std::to_string(counter++) + ".ply", std::vector{ Line{ oldPos, result.end } });
}
