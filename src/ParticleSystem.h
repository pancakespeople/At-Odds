#pragma once

#include <SFML/Graphics.hpp>

class ParticleSystem {
public:
	struct Particle {
		int life = 1000;
		sf::Vector2f velocity;
	};

	void createParticle(const Particle& particle, const sf::Vector2f& pos, const sf::Color& color);
	void updateParticles();
	void drawParticles(sf::RenderWindow& window);
	void createProjectileHitParticle(const sf::Vector2f& pos);

private:
	std::vector<Particle> m_particles;
	std::vector<sf::Vertex> m_particleVertices;
};