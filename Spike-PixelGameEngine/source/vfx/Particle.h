#pragma once

#include "../olcPixelGameEngine.h"

namespace vfx {
	struct Particle {
		float lifespan;
		float elapsed;
		olc::vf2d velocity;
		olc::vf2d position;

		uint8_t r;
		uint8_t g;
		uint8_t b;
	};
}