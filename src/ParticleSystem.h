#pragma once

#include <SFML/Graphics.hpp>

class Renderer;

class ParticleSystem {
public:
	const static inline int particleLimit = 10000;

	struct Particle {
		int life = 1000;
		sf::Vector2f velocity;
	};

	void createParticle(const Particle& particle, const sf::Vector2f& pos, const sf::Color& color);
	void updateParticles();
	void drawParticles(Renderer& renderer);
	void createProjectileHitParticle(const sf::Vector2f& pos);
	void createParticleExplosion(const sf::Vector2f& pos, const sf::Color& color, float velocity, int numParticles);

private:
	std::vector<Particle> m_particles;
	std::vector<sf::Vertex> m_particleVertices;
};