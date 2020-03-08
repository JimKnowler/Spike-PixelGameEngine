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
			float duration;
			std::vector<TimelineElement> timeline;
		};

		Level(const Config& config);

		void restart();

		/// @return true while playing, false when the level has ended
		bool update(float fElapsedTime);

		/// @brief get elapsed time since the start of the level
		float getProgress() const;

		/// @brief get the duration of the level
		float getDuration() const;

	private:
		Config config;

		float progress;				// elapsed time since the start of the level
		int timelineIndex;			// index of next entry in timeline to start
	};
}