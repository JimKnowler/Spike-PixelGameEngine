#pragma once

#include "../olcPixelGameEngine.h"

#include <functional>

namespace game {
    struct EnemyPath {
		/// @brief a callback that converts the time that has elapsed since the enemy was spawned,
		//         into a screen position for the enemy
		/// @param fElapsedTimeSinceSpawn The time that has elapsed since the enemy was spawned
		/// @param outPosition The position of the enemy
		/// @return true while the enemy is active, false when the enemy has gone off screen
		typedef std::function<bool(float fElapsedTimeSinceSpawn, olc::vf2d& outPosition)> CallbackPathPosition;

        CallbackPathPosition getPosition;
    };
}
