#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGE_GAMEPAD
#include "olcPGEX_Gamepad.h"

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

	struct Collidable {
		olc::vf2d position;

		virtual olc::vi2d getDimensions() const = 0;

		bool hasCollidedWith(const Collidable& other) const {
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

	struct CollidableSprite : public Collidable {
		olc::Sprite* sprite;

	public: // Collidable
		olc::vi2d getDimensions() const override {
			if (sprite) {
				return { sprite->width, sprite->height };
			}
			else {
				return { 0, 0 };
			}
		}
	};

	struct Player : public CollidableSprite {
		Player() : timeSinceLastShot(0.0f), state(State::ALIVE) {}

		float timeSinceLastShot;

		enum class State {
			ALIVE,
			DEAD
		};

		State state;
	};

	Player player;
	

	
	struct Star {
		olc::vf2d position;
		olc::Pixel pixel;
		float speed;
		int size;
	};
	
	Star stars[kNumStars];

	struct Bullet : public Collidable {
		olc::vf2d velocity;

	public:
		// Collidable
		olc::vi2d getDimensions() const override {
			return { kBulletWidth, kBulletHeight };
		}
	};

	std::list<Bullet> bullets;

	struct Enemy : public CollidableSprite {
		
	};

	std::list<Enemy> enemies;

	struct Game {
		Game() : score(0) {}

		int score;
	};

	Game game;

	struct Particle {
		float lifespan;
		float elapsed;
		olc::vf2d velocity;
		olc::vf2d position;
	};

	std::list<Particle> particles;

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

		UpdatePlayer(fElapsedTime);

		UpdateBullets(fElapsedTime);
	}

	void UpdatePlayer(float fElapsedTime) {
		if (player.state != Player::State::ALIVE) {
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
				player.state = Player::State::DEAD;

				// spawn particles
				olc::vf2d center = player.position + (player.getDimensions() / 2);

				SpawnParticles(center);
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
			std::remove_if(bullets.begin(), bullets.end(), [&](const Bullet& bullet) -> bool { return IsBulletOffscreen(bullet) || HasBulletCollided(bullet); }),
			bullets.end()
		);

		// update particles
		for (auto& particle : particles) {
			particle.elapsed += fElapsedTime;
			particle.position += (particle.velocity * fElapsedTime);
		}

		// remove stale particles
		particles.erase(
			std::remove_if(particles.begin(), particles.end(), [&](const Particle& particle) -> bool { return particle.elapsed >= particle.lifespan; }),
			particles.end()
		);
	}

	bool IsBulletOffscreen(const Bullet& bullet) {
		return bullet.position.y < 0;
	}

	bool HasBulletCollided(const Bullet& bullet) {
		for (auto it = enemies.begin(); it != enemies.end(); ) {
			const Enemy& enemy = *it;

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

	void OnEnemyDestroyed(const Enemy& enemy) {
		game.score += 1;

		olc::vf2d center = enemy.position + (olc::vf2d{ float(enemy.sprite->width), float(enemy.sprite->height) } * 0.5f);

		SpawnParticles(center);
	}

	void RenderGame() {
		// clear screen
		FillRect({ 0,0 }, { ScreenWidth(), ScreenHeight() }, olc::BLACK);
		
		// render stars
		for (auto& star : stars) {
			FillRect(star.position, {star.size, star.size}, star.pixel);
		}

		SetPixelMode(olc::Pixel::ALPHA);

		// render particles
		for (auto& particle : particles) {
			FillRect(particle.position, { 4, 4 }, olc::Pixel(255, 0, 255, 255 - uint8_t(255.0f * particle.elapsed / particle.lifespan)));
		}

		// render ship
		if (player.state == Player::State::ALIVE) {
			DrawSprite(player.position, player.sprite, 1);
		}

		// render enemies
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

		bullet.velocity = { 0, -kSpeedBullet };

		bullets.push_back(bullet);
	}

	void SpawnParticles(const olc::vf2d& position) {
		
		// starting at position

		for (int i = 0; i < 100; i++) {

			// random velocity
			olc::vf2d velocity = olc::vf2d{ float(rand() % 0x7ff) - 0x7ff/2.0f, float(rand() % 0x7ff) - 0x7ff/2.0f } * (400.0f / float(0x7ff));

			// random lifespan
			float lifespan = (rand() % 0x7ff) * ( 3.0f / float(0x7ff));

			// fading out over time

			Particle particle;
			particle.position = position;
			particle.lifespan = lifespan;
			particle.elapsed = 0.0f;
			particle.velocity = velocity;

			particles.push_back(particle);
		}
	}
};


int main()
{
	SpikePixelGame demo;
	if (demo.Construct(kScreenWidth, kScreenHeight, 2, 2))
		demo.Start();

	return 0;
}
