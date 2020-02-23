#pragma once

#include "Star.h"

namespace vfx {
	class StarField {
	public:
		void init(olc::PixelGameEngine& renderer) {
			const int32_t nScreenWidth = renderer.ScreenWidth();
			const int32_t nScreenHeight = renderer.ScreenHeight();

			for (auto& star : stars) {
				star.position = { float(rand() % nScreenWidth), float(rand() % nScreenHeight) };
				switch (rand() % 3) {
				case 0:
					star.pixel = olc::Pixel(255, 255, 255);
					star.speed = 50.0f + float(rand() % 50);
					star.size = 3;
					break;
				case 1:
					star.pixel = olc::Pixel(180, 180, 180);
					star.speed = 20.0f + float(rand() % 20);
					star.size = 2;
					break;
				case 2:
					star.pixel = olc::Pixel(100, 100, 100);
					star.speed = 10.0f + float(rand() % 10);
					star.size = 1;
					break;
				}
			}
		}

		void update(olc::PixelGameEngine& renderer, float fElapsedTime) {
			const int32_t nScreenHeight = renderer.ScreenHeight();
			const float fScreenHeight = float(nScreenHeight);

			for (auto& star : stars) {
				star.position.y += star.speed * fElapsedTime;

				if (star.position.y > nScreenHeight) {
					star.position.y -= fScreenHeight;
				}
			}
		}

		void render(olc::PixelGameEngine& renderer) {
			// render stars
			for (auto& star : stars) {
				renderer.FillRect(star.position, { star.size, star.size }, star.pixel);
			}
		}

	private:
		enum {
			kNumStars = 100
		};

		vfx::Star stars[kNumStars];
	};
}
