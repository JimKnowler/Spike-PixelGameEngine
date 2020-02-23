#pragma once

#include "../olcPixelGameEngine.h"

namespace vfx {
	struct Star {
		olc::vf2d position;
		olc::Pixel pixel;
		float speed;
		int size;
	};
}
