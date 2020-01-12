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
	const uint32_t kScreenWidth = 600;
	const uint32_t kScreenHeight = 400;
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

	struct Enemy {
		olc::Sprite* sprite;
		olc::vf2d position;
	};

	std::vector<Enemy> enemies;

	struct Game {
		Game() : score(0) {}

		int score;
	};

	Game game;

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

		CreatePlayer();

		CreateStars();

		CreateEnemies();

		CreateGame();

		return true;
	}

	void CreatePlayer() {
		player.sprite = new olc::Sprite("gfx//arwing_40pix.png");
		player.position = { (kScreenWidth - player.sprite->width) / 2.0f, float(kScreenHeight - (player.sprite->width * 2)) };
	}

	void CreateStars() {
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
	}

	void CreateEnemies() {
		const int kNumEnemiesX = 10;
		const int kNumEnemiesY = 4;

		const float kEnemySpacingX = float(kScreenWidth / (kNumEnemiesX + 1));
		const float kEnemySpacingY = 50.0f;
		const float kEnemyStartY = 30.0f;

		for (int x = 0; x < kNumEnemiesX; x++) {
			for (int y = 0; y < kNumEnemiesY; y++) {
				Enemy enemy;
				enemy.sprite = new olc::Sprite("gfx//enemy_40pix.png");
				enemy.position = { kEnemySpacingX * (x + 1), kEnemyStartY + (kEnemySpacingY * y) };
				enemy.position.x -= (enemy.sprite->width / 2.0f);
				enemies.push_back(enemy);
			}
		}
	}

	void CreateGame() {
		game.score = 0;
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

		// detect overlapping bullets + remove them
		for (auto it = bullets.begin(); it != bullets.end(); ) {
			if (HasBulletCollided(*it)) {
				it = bullets.erase(it);
			}
			else {
				it++;
			}
		}
	}

	bool HasBulletCollided(const Bullet& bullet) {
		for (auto it = enemies.begin(); it != enemies.end(); ) {
			const Enemy& enemy = *it;

			if (HasBulletCollidedWithEnemy(bullet, enemy)) {
				enemies.erase(it);
				game.score += 1;

				return true;
			}
			else {
				it++;
			}
		}

		return false;
	}

	bool HasBulletCollidedWithEnemy(const Bullet& bullet, const Enemy& enemy) {
		if ((bullet.position.x + kBulletWidth) < enemy.position.x) return false;
		if (bullet.position.x > (enemy.position.x + enemy.sprite->width)) return false;
		if ((bullet.position.y + kBulletHeight) < enemy.position.y) return false;
		if (bullet.position.y > (enemy.position.y + enemy.sprite->height)) return false;

		return true;
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

		// render enemies
		SetPixelMode(olc::Pixel::ALPHA);

		for (auto& enemy : enemies) {
			DrawSprite(enemy.position, enemy.sprite);
		}
		SetPixelMode(olc::Pixel::NORMAL);


		// render bullets
		for (auto& bullet : bullets) {
			FillRect(bullet.position, { kBulletWidth, kBulletHeight }, olc::Pixel(255,0,0));
		}

		// render score
		char buffer[32];
		sprintf_s(buffer, "%d", game.score);
		std::string strScore = "score: ";
		strScore += buffer;

		DrawString({ 10,10 }, strScore);
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
	if (demo.Construct(kScreenWidth, kScreenHeight, 2, 2))
		demo.Start();

	return 0;
}
