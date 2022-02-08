#pragma once

#include <SFML/Graphics.hpp>

class Renderer;

class ParticleSystem {
public:
	const static inline int particleLimit = 10000;

	struct Particle {
		Particle(int life, sf::Vector2f velocity) {
			this->life = life;
			this->initialLife = life;
			this->velocity = velocity;
		}

		int life = 1000;
		sf::Vector2f velocity;
		int initialLife = 1000;
	};

	void createParticle(const Particle& particle, const sf::Vector2f& pos, const sf::Color& color, float size);
	void updateParticles();
	void drawParticles(Renderer& renderer);
	void createProjectileHitParticle(const sf::Vector2f& pos);
	void createParticleExplosion(const sf::Vector2f& pos, float velocity, int numParticles, float size);

private:
	std::vector<Particle> m_particles;
	std::vector<sf::Vertex> m_particleVertices;
};