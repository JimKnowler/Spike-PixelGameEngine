#pragma once

#include "../olcPixelGameEngine.h"

#include "Particle.h"

namespace vfx {
	class ParticleSystem {
	public:
		void update(float fElapsedTime) {
			// update particles
			for (auto& particle : particles) {
				particle.elapsed += fElapsedTime;
				particle.position += (particle.velocity * fElapsedTime);
			}

			// remove stale particles
			particles.erase(
				std::remove_if(particles.begin(), particles.end(), [&](const vfx::Particle& particle) -> bool { return particle.elapsed >= particle.lifespan; }),
				particles.end()
			);
		}

		void render(olc::PixelGameEngine& renderer) {
			// render particles
			for (auto& particle : particles) {
				renderer.FillRect(particle.position, { 4, 4 }, olc::Pixel(255, 0, 255, 255 - uint8_t(255.0f * particle.elapsed / particle.lifespan)));
			}
		}

		void spawnExplosion(const olc::vf2d& position) {
			for (int i = 0; i < 100; i++) {

				// random velocity
				
				const float MATH_PI = 3.142f;
				const float angle = MATH_PI * 2.0f * randNormalisedFloat();
				const float speed = float(rand() % 400);
				
				olc::vf2d velocity = olc::vf2d{
					cosf(angle) * speed,
					sinf(angle) * speed
				};

				// random lifespan
				float lifespan = 3.0f * randNormalisedFloat();

				// fading out over time

				vfx::Particle particle;
				particle.position = position;
				particle.lifespan = lifespan;
				particle.elapsed = 0.0f;
				particle.velocity = velocity;

				particles.push_back(particle);
			}
		}
	private:
		float randNormalisedFloat() {
			// floating point value between 0.0f and 1.0f
			return float(rand()) / float(RAND_MAX);
		}

		std::list<Particle> particles;

	};
}