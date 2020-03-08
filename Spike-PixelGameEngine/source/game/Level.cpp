#include "game/Level.h"

namespace game {
	Level::Level(const Config& config) {

	}

	void Level::restart() {
		progress = 0.0f;
	}

	void Level::update(float fElapsedTime) {
		progress += fElapsedTime;
	}

	float Level::getProgress() const {
		return progress;
	}
}