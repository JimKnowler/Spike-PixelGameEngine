#pragma once

#include "game/EnemyPath.h"

// A collection of enemies that all follow the same path.
// A score bonus that is received if all enemies from the wave are destroyed
// Expires when all enemies have either been destroyed or have left the screen

namespace game {
	class EnemyWave {
	public:
		struct Config {
			EnemyPath path;					// path that enemies should follow
			int numEnemies;					// number of enemies to spawn
			float spawnInterval;			// time interval in between spawning enemies

			// todo: type of enemy (sprite/size/strength), bonus for destroying all enemies
		};

		void start();						// start spawning enemies for this wave

		/// @brief return true while running
		/// @brief return false when finished (all enemies offscreen or destroyed)
		bool update(float fElapsedTime);

	private:
		Config config;

		// todo: references to enemies that haven't been destroyed yet, and time since they were spawned
	};
}
