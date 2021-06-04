#include "gamepch.h"
#include "Planet.h"
#include "Random.h"

Planet::Planet(sf::Vector2f pos) {
	m_shape.setPosition(pos);
	m_shape.setFillColor(sf::Color(155, 155, 155));
	m_shape.setRadius(250.0f);
	m_shape.setOrigin(sf::Vector2f(250.0f, 250.0f));

	m_shaderRandomSeed = Random::randFloat(0.0f, 1.0f);
}

void Planet::draw(sf::RenderWindow& window, EffectsEmitter& emitter) {
	emitter.drawGlow(window, m_shape.getPosition(), 750.0f);
	emitter.drawPlanet(window, m_shape, m_shaderRandomSeed);
}