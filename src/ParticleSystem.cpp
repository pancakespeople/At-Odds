#include "gamepch.h"
#include "ParticleSystem.h"
#include "Random.h"

void ParticleSystem::createParticle(const Particle& particle, const sf::Vector2f& pos, const sf::Color& color) {
	m_particles.push_back(particle);

	sf::Vertex v0;
	v0.position.x = pos.x;
	v0.position.y = pos.y;
	v0.color = color;

	sf::Vertex v1;
	v1.position.x = pos.x;
	v1.position.y = pos.y + 10.0f;
	v1.color = color;

	sf::Vertex v2;
	v2.position.x = pos.x + 10.0f;
	v2.position.y = pos.y + 10.0f;
	v2.color = color;

	sf::Vertex v3;
	v3.position.x = pos.x + 10.0f;
	v3.position.y = pos.y;
	v3.color = color;

	m_particleVertices.push_back(v0);
	m_particleVertices.push_back(v1);
	m_particleVertices.push_back(v2);
	m_particleVertices.push_back(v3);
}

void ParticleSystem::updateParticles() {
	for (size_t i = 0; i < m_particles.size(); i++) {
		m_particleVertices[i * 4].position += m_particles[i].velocity;
		m_particleVertices[i * 4 + 1].position += m_particles[i].velocity;
		m_particleVertices[i * 4 + 2].position += m_particles[i].velocity;
		m_particleVertices[i * 4 + 3].position += m_particles[i].velocity;

		m_particleVertices[i * 4].color.a = static_cast<sf::Uint8>(255.0 * (m_particles[i].life / 1000.0));
		m_particleVertices[i * 4 + 1].color.a = static_cast<sf::Uint8>(255.0 * (m_particles[i].life / 1000.0));
		m_particleVertices[i * 4 + 2].color.a = static_cast<sf::Uint8>(255.0 * (m_particles[i].life / 1000.0));
		m_particleVertices[i * 4 + 3].color.a = static_cast<sf::Uint8>(255.0 * (m_particles[i].life / 1000.0));

		if (m_particles[i].life == 0) {
			m_particles.erase(m_particles.begin() + i);
			for (int j = 0; j < 4; j++) {
				m_particleVertices.erase(m_particleVertices.begin() + (i * 4));
			}
			i--;
		}
		else {
			m_particles[i].life--;
		}
	}
}

void ParticleSystem::drawParticles(sf::RenderWindow& window) {
	if (m_particleVertices.size() > 0) {
		window.draw(&m_particleVertices[0], m_particleVertices.size(), sf::Quads);
	}
}

void ParticleSystem::createProjectileHitParticle(const sf::Vector2f& pos) {
	createParticle(Particle{ 1000, Random::randVec(-10.0f, 10.0f) }, pos, sf::Color(255, 165, 0));
}