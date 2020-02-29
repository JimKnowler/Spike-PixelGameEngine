#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGE_GAMEPAD
#include "olcPGEX_Gamepad.h"

#include <algorithm>

#include "actor/Player.h"
#include "actor/Bullet.h"
#include "actor/Enemy.h"

#include "vfx/StarField.h"
#include "vfx/ParticleSystem.h"

namespace {
	const float kSpeedShip = 300;
	const float kSpeedBullet = 1000.0f;
	const float kMinTimeBetweenBullets = 0.1f;
	const uint32_t kScreenWidth = 800;
	const uint32_t kScreenHeight = 600;

	const char* kFilePathPlayer = "gfx//arwing_40pix.png";
	const char* kFilePathEnemy = "gfx//enemy_40pix.png";

	const float kGameDuration = 240.0f;

	const int kMaxScore = 50;
}

class SpikePixelGame : public olc::PixelGameEngine
{
public:
	SpikePixelGame()
	{
		sAppName = "Spike Pixel Game";
	}

	actor::Player player;
	vfx::StarField starField;
	std::list<actor::Bullet> bullets;
	std::list<actor::Enemy> enemies;


	struct Game {
		Game() {}

		int score;
		float progress;
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

		Restart();

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
		player.sprite = new olc::Sprite(kFilePathPlayer);
	}

	void CreateStars() {
		starField.init(*this);
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
				enemy.sprite = new olc::Sprite(kFilePathEnemy);
				enemy.position = { kEnemySpacingX * (x + 1), kEnemyStartY + (kEnemySpacingY * y) };
				enemy.position.x -= (enemy.sprite->width / 2.0f);
				enemies.push_back(enemy);
			}
		}
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		UpdateGame(fElapsedTime);

		RenderGame();

		return true;
	}

	void Restart() {
		enemies.clear();
		CreateEnemies();

		player.position = { (kScreenWidth - player.sprite->width) / 2.0f, float(kScreenHeight - (player.sprite->width * 2)) };
		player.state = actor::Player::State::ALIVE;

		game.score = 0;
		game.progress = 0;
	}

	void UpdateGame(float fElapsedTime) {
		if (GetKey(olc::ESCAPE).bPressed) {
			Restart();
		}

		game.progress += fElapsedTime;
		
		if (game.progress > kGameDuration) {
			// end of the game
			Restart();
		}

		const float progressNormalised = float(std::min(game.score, kMaxScore)) / float(kMaxScore);
		
		float starFieldSpeed = 1.0f + (9.0f * progressNormalised);

		starField.update(*this, fElapsedTime * starFieldSpeed);

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
		
		// set dx,dy as normalised directional vector 
		float dx = 0.0f;
		float dy = 0.0f;

		if (GetKey(olc::LEFT).bHeld || gamepad.getButton(olc::GPButtons::DPAD_L).bHeld) {
			dx = -1;
		} else if (GetKey(olc::RIGHT).bHeld || gamepad.getButton(olc::GPButtons::DPAD_R).bHeld) {
			dx = 1;
		} else {
			dx = gamepad.getAxis(olc::GPAxes::LX);
		}

		if (GetKey(olc::UP).bHeld || gamepad.getButton(olc::GPButtons::DPAD_U).bHeld) {
			dy = -1;
		} else if (GetKey(olc::DOWN).bHeld || gamepad.getButton(olc::GPButtons::DPAD_D).bHeld) {
			dy = 1;
		} else {
			dy = gamepad.getAxis(olc::GPAxes::LY);
		}

		// multiple normalised value by itself to get exponential response curve
		dx *= fabs(dx);
		dy *= fabs(dy);

		// update the player position
		player.position.x += dx * kSpeedShip * fElapsedTime;
		player.position.y += dy * kSpeedShip * fElapsedTime;

		// clamp player position to the visible screen
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

		// fire a bullet
		if (GetKey(olc::SPACE).bHeld || gamepad.getButton(olc::GPButtons::FACE_D).bHeld) {
			if (player.timeSinceLastShot >= kMinTimeBetweenBullets) {
				player.timeSinceLastShot = 0.0f;

				EmitPlayerBullet();
			}
		}
	}

	void UpdateBullets(float fElapsedTime) {		
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
			} else {
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
		
		starField.render(*this);

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

		// render progress
		DrawString({ 10,20 }, "progress: ");
		const int kBarFrameWidth = kScreenWidth - 122;
		const int kBarWidth = int( float(kBarFrameWidth) * game.progress / kGameDuration);
		DrawRect({ 100,20 }, { kBarFrameWidth + 2, 10 }, olc::WHITE);
		FillRect({ 102,21 }, { kBarWidth, 8 }, olc::RED);
	}

	void EmitPlayerBullet() {
		actor::Bullet bullet;

		// position that bullets start from
		olc::vf2d position = player.position;

		// adjust position for center of ship
		const float kHalfPlayerWidth = float(player.sprite->width / 2);
		position.x += kHalfPlayerWidth;

		// adjust position for center of bullet
		position.x -= bullet.getDimensions().x / 2;

		if (true) {
			// bullet going forward
			bullet.position = position;
			bullet.velocity = { 0, -kSpeedBullet };
			bullets.push_back(bullet);
		}

		if (false) {
			// left & right secondary bullets going forward
			bullet.position = position;
			bullet.position.x += kHalfPlayerWidth / 2;
			bullets.push_back(bullet);

			bullet.position = position;
			bullet.position.x -= kHalfPlayerWidth / 2;
			bullets.push_back(bullet);
		}

		if (false) {
			// left & right diagonal bullets
			bullet.position = position;
			
			bullet.velocity = { -kSpeedBullet, -kSpeedBullet };
			bullets.push_back(bullet);
			
			bullet.velocity = { kSpeedBullet, -kSpeedBullet };
			bullets.push_back(bullet);
		}
	}
};

int main()
{
	SpikePixelGame demo;
	if (demo.Construct(kScreenWidth, kScreenHeight, 1, 1))
		demo.Start();

	return 0;
}
