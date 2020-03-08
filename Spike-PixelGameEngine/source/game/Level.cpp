#include "game/Level.h"

namespace game {
	Level::Level(const Config& inConfig) : config(inConfig) {

	}

	void Level::restart() {
		progress = 0.0f;
		timelineIndex = 0;
	}

	bool Level::update(float fElapsedTime) {
		progress += fElapsedTime;

		return progress < config.duration;
	}

	float Level::getProgress() const {
		return progress;
	}

	float Level::getDuration() const {
		return config.duration;
	}
}