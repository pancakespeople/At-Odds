#include "gamepch.h"
#include "AsteroidBelt.h"
#include "Random.h"
#include "Renderer.h"
#include "Math.h"
#include "TextureCache.h"
#include "Star.h"

const std::vector<std::vector<sf::Vector2f>> sheetTexCoords = {
	{{0.0f, 0.0f}, {0.0f, 0.5f}, {0.33f, 0.5f}, {0.33f, 0.0f}},
	{{0.33f, 0.0f}, {0.33f, 0.5f}, {0.66f, 0.5f}, {0.66f, 0.0f}},
	{{0.66f, 0.0f}, {0.66f, 0.5f}, {1.0f, 0.5f}, {1.0f, 0.0f}},
	{{0.0f, 0.5f}, {0.0f, 1.0f}, {0.33f, 1.0f}, {0.33f, 0.5f}},
	{{0.33f, 0.5f}, {0.33f, 1.0f}, {0.66f, 1.0f}, {0.66f, 0.5f}},
	{{0.66f, 0.5f}, {0.66f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.5f}},
};

void AsteroidBelt::generate(const Star& star) {
	sf::Clock timer;
	
	std::string genState = Random::getGeneratorState();
	Random::setGeneratorSeed(m_seed);

	int numAsteroids = Random::randInt(5000, 12500);
	float radius = Random::randFloat(500.0f, 2000.0f);
	float aspect = 640.0f / 360.0f;

	m_vertices.reserve(numAsteroids);
	for (int i = 0; i < numAsteroids; i++) {
		sf::Vector2f pos = Math::normalize(Random::randPointInCircle(1.0f)) * (m_radius + Random::randFloat(-radius, radius));
		float size = Random::randFloat(25.0f, 100.0f);
		std::vector<sf::Vector2f> texCoords = sheetTexCoords[Random::randInt(0, sheetTexCoords.size() - 1)];
		sf::Color color;

		// Vertex colors
		if (Random::randFloat(0.0f, 1.0f) > 0.1f) {
			int col = Random::randInt(125, 255);
			color = sf::Color( col, col, col, 255 );
		}
		else {
			float rnd = Random::randFloat(0.0f, 1.0f);

			if (rnd > 0.5f) {
				// 50% chance
				color = sf::Color::Red;
			}
			else if (rnd > 0.15f) {
				// 35% chance
				color = sf::Color(0, 255, 255);
			}
			else {
				// 15% chance
				color = sf::Color::Yellow;
			}
		}

		// Bottom left
		sf::Vertex v1;
		v1.position = pos + sf::Vector2f{ -size*aspect, -size };
		v1.texCoords = texCoords[0];
		//v1.texCoords = { 0.0f, 0.0f };
		v1.color = color;

		// Top left
		sf::Vertex v2;
		v2.position = pos + sf::Vector2f{ -size*aspect, size };
		v2.texCoords = texCoords[1];
		//v2.texCoords = { 0.0f, 1.0f };
		v2.color = color;

		// Top right
		sf::Vertex v3;
		v3.position = pos + sf::Vector2f{ size*aspect, size };
		v3.texCoords = texCoords[2];
		//v3.texCoords = { 1.0f, 1.0f };
		v3.color = color;

		// Bottom right
		sf::Vertex v4;
		v4.position = pos + sf::Vector2f{ size*aspect, -size };
		v4.texCoords = texCoords[3];
		//v4.texCoords = { 1.0f, 0.0f };
		v4.color = color;

		m_vertices.push_back(v1);
		m_vertices.push_back(v2);
		m_vertices.push_back(v3);
		m_vertices.push_back(v4);
	}

	DEBUG_PRINT("Generated an asteroid belt with " << m_vertices.size() << " vertices " << "in " << timer.getElapsedTime().asMilliseconds() << " ms");
	m_generated = true;

	Random::setGeneratorState(genState);
}

void AsteroidBelt::draw(Renderer& renderer, const Star& star) {
	renderer.effects.drawAsteroidBelt(m_vertices, 500.0f/m_radius, star.getLocalViewCenter());
}