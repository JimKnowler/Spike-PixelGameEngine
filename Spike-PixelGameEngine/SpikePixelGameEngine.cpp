#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <algorithm>

namespace {
	const uint32_t kNumStars = 100;
	const float kSpeedShip = 200.0f;
	const float kSpeedBullet = 1000.0f;
	const int32_t kBulletWidth = 6;
	const int32_t kBulletHeight = 6;
	const float kMinTimeBetweenBullets = 0.1f;
}

class SpikePixelGame : public olc::PixelGameEngine
{
public:
	SpikePixelGame()
	{
		sAppName = "Spike Pixel Game";
	}

	struct Player {
		Player() : timeSinceLastShot(0.0f) {}

		olc::Sprite* sprite;
		olc::vf2d position;
		float timeSinceLastShot;
	};

	Player player;
	
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

		player.sprite = new olc::Sprite("gfx//arwing_40pix.png");
		player.position = { 100, 100 };

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
			player.position.x -= kSpeedShip * fElapsedTime;
		}

		if (GetKey(olc::RIGHT).bHeld) {
			player.position.x += kSpeedShip * fElapsedTime;
		}

		if (GetKey(olc::UP).bHeld) {
			player.position.y -= kSpeedShip * fElapsedTime;
		}

		if (GetKey(olc::DOWN).bHeld) {
			player.position.y += kSpeedShip * fElapsedTime;
		}

		player.position.x = std::clamp(player.position.x, 0.0f, float(ScreenWidth() - player.sprite->width));
		player.position.y = std::clamp(player.position.y, 0.0f, float(ScreenHeight() - player.sprite->height));


		// update player
		player.timeSinceLastShot += fElapsedTime;

		// trigger bullets
		if (GetKey(olc::SPACE).bHeld) {
			if (player.timeSinceLastShot >= kMinTimeBetweenBullets) {
				player.timeSinceLastShot = 0.0f;

				EmitPlayerBullet();
			}
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
		DrawSprite(player.position, player.sprite, 1);
		SetPixelMode(olc::Pixel::NORMAL);

		// render bullets
		for (auto& bullet : bullets) {
			FillRect(bullet.position, { kBulletWidth, kBulletHeight }, olc::Pixel(255,0,0));
		}

	}

	void EmitPlayerBullet() {
		Bullet bullet;
		bullet.position = player.position;

		// adjust position for center of ship
		bullet.position.x += player.sprite->width / 2;

		// adjust position for center of bullet
		bullet.position.x -= kBulletWidth / 2;

		bullets.push_back(bullet);
	}
};


int main()
{
	SpikePixelGame demo;
	if (demo.Construct(500, 400, 2, 2))
		demo.Start();

	return 0;
}
