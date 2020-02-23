#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGE_GAMEPAD
#include "olcPGEX_Gamepad.h"

#include <algorithm>

#include "actor/Player.h"
#include "actor/Bullet.h"
#include "actor/Enemy.h"

#include "vfx/Star.h"
#include "vfx/ParticleSystem.h"

namespace {
	const uint32_t kNumStars = 100;
	const float kSpeedShip = 200.0f;
	const float kSpeedBullet = 1000.0f;
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

	actor::Player player;
	vfx::Star stars[kNumStars];
	std::list<actor::Bullet> bullets;
	std::list<actor::Enemy> enemies;

	struct Game {
		Game() : score(0) {}

		int score;
	};

	Game game;

	vfx::ParticleSystem particleSystem;
	olc::GamePad gamepad;
	std::vector<olc::GamePad> gamepads;
public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

		CreateGamePads();

		CreatePlayer();

		CreateStars();

		CreateEnemies();

		CreateGame();

		return true;
	}

	void CreateGamePads() {
		olc::GamePad::init();
		gamepads = olc::GamePad::getGamepads();
		if (!gamepads.empty()) {
			gamepad = gamepads[0];
		}
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
				actor::Enemy enemy;
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

		UpdatePlayer(fElapsedTime);

		UpdateBullets(fElapsedTime);
	}

	void UpdatePlayer(float fElapsedTime) {
		if (player.state != actor::Player::State::ALIVE) {
			return;
		}

		// update spaceship position
		if (gamepad.valid) {
			gamepad.poll();
		}
		
		if (GetKey(olc::LEFT).bHeld || gamepad.getButton(olc::GPButtons::DPAD_L).bHeld) {
			player.position.x -= kSpeedShip * fElapsedTime;
		}

		if (GetKey(olc::RIGHT).bHeld || gamepad.getButton(olc::GPButtons::DPAD_R).bHeld) {
			player.position.x += kSpeedShip * fElapsedTime;
		}

		if (GetKey(olc::UP).bHeld || gamepad.getButton(olc::GPButtons::DPAD_U).bHeld) {
			player.position.y -= kSpeedShip * fElapsedTime;
		}

		if (GetKey(olc::DOWN).bHeld || gamepad.getButton(olc::GPButtons::DPAD_D).bHeld) {
			player.position.y += kSpeedShip * fElapsedTime;
		}

		player.position.x = std::clamp(player.position.x, 0.0f, float(ScreenWidth() - player.sprite->width));
		player.position.y = std::clamp(player.position.y, 0.0f, float(ScreenHeight() - player.sprite->height));

		// update player shooting limiter
		player.timeSinceLastShot += fElapsedTime;

		// check collision of player with enemy
		for (auto& enemy : enemies) {
			if (enemy.hasCollidedWith(player)) {
				player.state = actor::Player::State::DEAD;

				// spawn particles
				olc::vf2d center = player.position + (player.getDimensions() / 2);

				particleSystem.spawnExplosion(center);
			}
		}
	}

	void UpdateBullets(float fElapsedTime) {
		// trigger bullets
		if (GetKey(olc::SPACE).bHeld || gamepad.getButton(olc::GPButtons::FACE_D).bHeld) {
			if (player.timeSinceLastShot >= kMinTimeBetweenBullets) {
				player.timeSinceLastShot = 0.0f;

				EmitPlayerBullet();
			}
		}
		
		// update position of bullets
		for (auto& bullet : bullets) {
			bullet.position += (bullet.velocity * fElapsedTime);
		}

		// remove offscreen bullets
		bullets.erase(
			std::remove_if(bullets.begin(), bullets.end(), [&](const actor::Bullet& bullet) -> bool { return IsBulletOffscreen(bullet) || HasBulletCollided(bullet); }),
			bullets.end()
		);

		particleSystem.update(fElapsedTime);
	}

	bool IsBulletOffscreen(const actor::Bullet& bullet) {
		return bullet.position.y < 0;
	}

	bool HasBulletCollided(const actor::Bullet& bullet) {
		for (auto it = enemies.begin(); it != enemies.end(); ) {
			const actor::Enemy& enemy = *it;

			if (bullet.hasCollidedWith(enemy)) {
				OnEnemyDestroyed(enemy);
				enemies.erase(it);

				return true;
			}
			else {
				it++;
			}
		}

		return false;
	}

	void OnEnemyDestroyed(const actor::Enemy& enemy) {
		game.score += 1;

		olc::vf2d center = enemy.position + (olc::vf2d{ float(enemy.sprite->width), float(enemy.sprite->height) } * 0.5f);

		particleSystem.spawnExplosion(center);
	}

	void RenderGame() {
		// clear screen
		FillRect({ 0,0 }, { ScreenWidth(), ScreenHeight() }, olc::BLACK);
		
		// render stars
		for (auto& star : stars) {
			FillRect(star.position, {star.size, star.size}, star.pixel);
		}

		SetPixelMode(olc::Pixel::ALPHA);

		particleSystem.render(*this);

		// render ship
		if (player.state == actor::Player::State::ALIVE) {
			DrawSprite(player.position, player.sprite, 1);
		}

		// render enemies
		for (auto& enemy : enemies) {
			DrawSprite(enemy.position, enemy.sprite);
		}

		SetPixelMode(olc::Pixel::NORMAL);


		// render bullets
		for (auto& bullet : bullets) {
			FillRect(bullet.position, bullet.getDimensions(), olc::Pixel(255,0,0));
		}

		// render score
		char buffer[32];
		sprintf_s(buffer, "%d", game.score);
		std::string strScore = "score: ";
		strScore += buffer;

		DrawString({ 10,10 }, strScore);
	}

	void EmitPlayerBullet() {
		actor::Bullet bullet;
		bullet.position = player.position;

		// adjust position for center of ship
		bullet.position.x += player.sprite->width / 2;

		// adjust position for center of bullet
		bullet.position.x -= bullet.getDimensions().x / 2;

		bullet.velocity = { 0, -kSpeedBullet };

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