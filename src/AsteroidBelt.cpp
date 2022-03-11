#include "gamepch.h"
#include "AsteroidBelt.h"
#include "Random.h"
#include "Renderer.h"
#include "Math.h"
#include "TextureCache.h"
#include "Star.h"

void AsteroidBelt::generate(const Star& star) {
	std::string genState = Random::getGeneratorState();
	Random::setGeneratorSeed(m_seed);

	m_vertices.reserve(10000);
	for (int i = 0; i < 10000; i++) {
		sf::Vector2f pos = Math::normalize(Random::randPointInCircle(1.0f)) * (m_radius + Random::randFloat(-1000.0f, 1000.0f));
		float aspect = 640.0f / 360.0f;
		float size = Random::randFloat(25.0f, 100.0f);

		// Bottom left
		sf::Vertex v1;
		v1.position = pos + sf::Vector2f{ -size*aspect, -size };
		v1.texCoords = { 0.0f, 0.0f };

		// Top left
		sf::Vertex v2;
		v2.position = pos + sf::Vector2f{ -size*aspect, size };
		v2.texCoords = { 0.0f, 1.0f };

		// Top right
		sf::Vertex v3;
		v3.position = pos + sf::Vector2f{ size*aspect, size };
		v3.texCoords = { 1.0f, 1.0f };

		// Bottom right
		sf::Vertex v4;
		v4.position = pos + sf::Vector2f{ size*aspect, -size };
		v4.texCoords = { 1.0f, 0.0f };

		m_vertices.push_back(v1);
		m_vertices.push_back(v2);
		m_vertices.push_back(v3);
		m_vertices.push_back(v4);
	}

	DEBUG_PRINT("Generated an asteroid belt with " << m_vertices.size() << " vertices");
	m_generated = true;

	Random::setGeneratorState(genState);
}

void AsteroidBelt::draw(Renderer& renderer, const Star& star) {
	renderer.effects.drawAsteroidBelt(m_vertices, 500.0f/m_radius, star.getLocalViewCenter());
}