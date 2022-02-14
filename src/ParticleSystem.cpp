#include "gamepch.h"
#include "ParticleSystem.h"
#include "Random.h"
#include "Renderer.h"

void ParticleSystem::createParticle(const Particle& particle, const sf::Vector2f& pos, const sf::Color& color, float size) {
	if (m_particles.size() < particleLimit) {
		m_particles.push_back(particle);

		float mid = size / 2.0f;

		sf::Vertex v0;
		v0.position.x = pos.x - mid;
		v0.position.y = pos.y - mid;
		v0.color = color;
		v0.texCoords = { 0.0f, 0.0f };

		sf::Vertex v1;
		v1.position.x = pos.x - mid;
		v1.position.y = pos.y + size - mid;
		v1.color = color;
		v1.texCoords = { 0.0f, 1.0f };

		sf::Vertex v2;
		v2.position.x = pos.x + size - mid;
		v2.position.y = pos.y + size - mid;
		v2.color = color;
		v2.texCoords = { 1.0f, 1.0f };

		sf::Vertex v3;
		v3.position.x = pos.x + size - mid;
		v3.position.y = pos.y - mid;
		v3.color = color;
		v3.texCoords = { 1.0f, 0.0f };

		m_particleVertices.push_back(v0);
		m_particleVertices.push_back(v1);
		m_particleVertices.push_back(v2);
		m_particleVertices.push_back(v3);
	}
}

void ParticleSystem::updateParticles() {
	for (size_t i = 0; i < m_particles.size(); i++) {
		m_particleVertices[i * 4].position += m_particles[i].velocity;
		m_particleVertices[i * 4 + 1].position += m_particles[i].velocity;
		m_particleVertices[i * 4 + 2].position += m_particles[i].velocity;
		m_particleVertices[i * 4 + 3].position += m_particles[i].velocity;

		m_particleVertices[i * 4].color.a = static_cast<sf::Uint8>(255.0 * (m_particles[i].life / (float)m_particles[i].initialLife));
		m_particleVertices[i * 4 + 1].color.a = static_cast<sf::Uint8>(255.0 * (m_particles[i].life / (float)m_particles[i].initialLife));
		m_particleVertices[i * 4 + 2].color.a = static_cast<sf::Uint8>(255.0 * (m_particles[i].life / (float)m_particles[i].initialLife));
		m_particleVertices[i * 4 + 3].color.a = static_cast<sf::Uint8>(255.0 * (m_particles[i].life / (float)m_particles[i].initialLife));

		if (m_particles[i].life == 0) {
			/*m_particles.erase(m_particles.begin() + i);
			m_particleVertices.erase(m_particleVertices.begin() + (i * 4), m_particleVertices.begin() + (i * 4) + 4);*/
			
			if (i < m_particles.size() - 1) {
				std::swap(m_particles[i], m_particles[m_particles.size() - 1]);
				m_particles.pop_back();

				std::swap(m_particleVertices[i * 4], m_particleVertices[m_particleVertices.size() - 4]);
				std::swap(m_particleVertices[i * 4 + 1], m_particleVertices[m_particleVertices.size() - 3]);
				std::swap(m_particleVertices[i * 4 + 2], m_particleVertices[m_particleVertices.size() - 2]);
				std::swap(m_particleVertices[i * 4 + 3], m_particleVertices[m_particleVertices.size() - 1]);

				m_particleVertices.pop_back();
				m_particleVertices.pop_back();
				m_particleVertices.pop_back();
				m_particleVertices.pop_back();
			}
			else {
				m_particles.pop_back();

				m_particleVertices.pop_back();
				m_particleVertices.pop_back();
				m_particleVertices.pop_back();
				m_particleVertices.pop_back();
			}

			i--;
		}
		else {
			m_particles[i].life--;
		}
	}
}

void ParticleSystem::drawParticles(Renderer& renderer) {
	if (m_particleVertices.size() > 0) {
		renderer.effects.drawParticles(m_particleVertices);
	}
}

void ParticleSystem::createProjectileHitParticle(const sf::Vector2f& pos) {
	createParticle(Particle{ 1000, Random::randVec(-10.0f, 10.0f) }, pos, sf::Color(255, 165, 0), 10);
}

void ParticleSystem::createParticleExplosion(const sf::Vector2f& pos, float velocity, int numParticles, float size) {
	for (int i = 0; i < numParticles; i++) {
		createParticle(Particle{ 1000, Random::randVec(-velocity, velocity) }, pos, { 255, 204, 179 }, size);
	}
}