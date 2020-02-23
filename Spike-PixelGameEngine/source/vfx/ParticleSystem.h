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
				olc::vf2d velocity = olc::vf2d{ float(rand() % 0x7ff) - 0x7ff / 2.0f, float(rand() % 0x7ff) - 0x7ff / 2.0f } *(400.0f / float(0x7ff));

				// random lifespan
				float lifespan = (rand() % 0x7ff) * (3.0f / float(0x7ff));

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
		std::list<Particle> particles;

	};
}