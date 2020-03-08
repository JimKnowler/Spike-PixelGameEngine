#pragma once

#include "../olcPixelGameEngine.h"

#include <functional>

namespace game {
    struct EnemyPath {
        typedef std::function<olc::vf2d(float fElapsedTimeSinceSpawn)> CallbackPathPosition;

        CallbackPathPosition getPosition;
    };
}
