#pragma once

#include "CollidableAABB.h"

namespace collidable {
	struct CollidableSprite : public CollidableAABB {
		olc::Sprite* sprite;

	public: // CollidableAABB
		olc::vi2d getDimensions() const override {
			if (sprite) {
				return { sprite->width, sprite->height };
			}
			else {
				return { 0, 0 };
			}
		}
	};
}

