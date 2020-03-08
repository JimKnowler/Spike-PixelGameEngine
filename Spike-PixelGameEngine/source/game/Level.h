#pragma once

#include "game/EnemyWave.h"

namespace game {
	class Level {
	public:
		struct TimelineElement {
			float startTime;
			EnemyWave enemyWave;
		};

		struct Config {
			std::vector<TimelineElement> timeline;
		};

		Level(const Config& config);

		void restart();

		void update(float fElapsedTime);

		/// @brief get elapsed time since the start of the level
		float getProgress() const;

	private:
		float progress;
	};
}