#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <algorithm>

namespace {
	const uint32_t kNumStars = 100;
	const float kSpeedShip = 200.0f;
	const float kSpeedBullet = 1000.0f;
}

class SpikePixelGame : public olc::PixelGameEngine
{
public:
	SpikePixelGame()
	{
		sAppName = "Spike Pixel Game";
	}

	olc::Sprite* spriteShip;
	olc::vf2d positionShip;
	
	struct Star {
		olc::vf2d position;
		olc::Pixel pixel;
		float speed;
		int size;
	};
	
	Star stars[kNumStars];

	struct Bullet {
		olc::vf2d position;
	};

	std::vector<Bullet> bullets;

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

		spriteShip = new olc::Sprite("gfx//arwing_40pix.png");
		positionShip = { 100, 100 };

		for (auto& star : stars) {
			star.position = { float(rand() % ScreenWidth()), float(rand() % ScreenHeight()) };
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

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		UpdateGame(fElapsedTime);

		RenderGame();

		return true;
	}

	void UpdateGame(float fElapsedTime) {
		// animate stars
		for (auto& star : stars) {
			star.position.y += star.speed * fElapsedTime;
			if (star.position.y > ScreenHeight()) {
				star.position.y -= float(ScreenHeight());
			}
		}

		// update spaceship position
		if (GetKey(olc::LEFT).bHeld) {
			positionShip.x -= kSpeedShip * fElapsedTime;
		}

		if (GetKey(olc::RIGHT).bHeld) {
			positionShip.x += kSpeedShip * fElapsedTime;
		}

		if (GetKey(olc::UP).bHeld) {
			positionShip.y -= kSpeedShip * fElapsedTime;
		}

		if (GetKey(olc::DOWN).bHeld) {
			positionShip.y += kSpeedShip * fElapsedTime;
		}

		positionShip.x = std::clamp(positionShip.x, 0.0f, float(ScreenWidth() - spriteShip->width));
		positionShip.y = std::clamp(positionShip.y, 0.0f, float(ScreenHeight() - spriteShip->height));

		// trigger bullets
		if (GetKey(olc::SPACE).bPressed) {
			Bullet bullet;
			bullet.position = positionShip;
			// adjust position for center of ship
			bullet.position.x += spriteShip->width / 2;

			bullets.push_back(bullet);
		}

		// update position of bullets
		for (auto& bullet : bullets) {
			bullet.position.y -= kSpeedBullet * fElapsedTime;
		}

		// remove offscreen bullets
		for (auto it = bullets.begin(); it != bullets.end(); ) {
			if (it->position.y < 0) {
				it = bullets.erase(it);
			}
			else {
				it++;
			}
		}
	}

	void RenderGame() {
		// clear screen
		FillRect({ 0,0 }, { ScreenWidth(), ScreenHeight() }, olc::BLACK);
		
		// render stars
		for (auto& star : stars) {
			FillRect(star.position, {star.size, star.size}, star.pixel);
		}

		// render ship
		SetPixelMode(olc::Pixel::ALPHA);
		DrawSprite(positionShip, spriteShip, 1);
		SetPixelMode(olc::Pixel::NORMAL);

		// render bullets
		for (auto& bullet : bullets) {
			FillRect(bullet.position, { 6, 6 }, olc::Pixel(255,0,0));
		}

	}
};


int main()
{
	SpikePixelGame demo;
	if (demo.Construct(500, 400, 2, 2))
		demo.Start();

	return 0;
}
