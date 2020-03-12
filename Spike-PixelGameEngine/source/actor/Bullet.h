#pragma once

#include "../collidable/CollidableAABB.h"

namespace actor {
	struct Bullet : public collidable::CollidableAABB {
		olc::vf2d velocity;

		olc::Pixel colour;

	public: // CollidableAABB
		olc::vi2d getDimensions() const override {
			const int32_t kBulletWidth = 6;
			const int32_t kBulletHeight = 6;

			return { kBulletWidth, kBulletHeight };
		}
	};
}