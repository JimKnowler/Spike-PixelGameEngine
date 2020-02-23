#pragma once

#include "../collidable/CollidableSprite.h"

namespace actor {
	struct Player : public collidable::CollidableSprite {
		Player() : timeSinceLastShot(0.0f), state(State::ALIVE) {}

		float timeSinceLastShot;

		enum class State {
			ALIVE,
			DEAD
		};

		State state;
	};
}
