#pragma once

#include "../olcPixelGameEngine.h"

namespace collidable {
	struct CollidableAABB {
		olc::vf2d position;

		virtual olc::vi2d getDimensions() const = 0;

		bool hasCollidedWith(const CollidableAABB& other) const {
			olc::vi2d dimensions = getDimensions();
			olc::vi2d dimensionsOther = other.getDimensions();
			olc::vf2d positionOther = other.position;

			// AABB collision

			if ((position.x + dimensions.x) < positionOther.x) return false;
			if (position.x > (positionOther.x + dimensionsOther.x)) return false;
			if ((position.y + dimensions.y) < positionOther.y) return false;
			if (position.y > (positionOther.y + dimensionsOther.y)) return false;

			return true;
		}
	};
}