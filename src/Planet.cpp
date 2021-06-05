#include "gamepch.h"
#include "Planet.h"
#include "Random.h"
#include "Math.h"

Planet::Planet(sf::Vector2f pos, sf::Vector2f starPos) {
	m_shape.setFillColor(sf::Color(155, 155, 155));
	m_shape.setRadius(250.0f);
	m_shape.setOrigin(sf::Vector2f(250.0f, 250.0f));
	m_shape.setPosition(pos);

	float radiusFromStar = Math::distance(pos, starPos);
	float orbitSpeed = 1000.0f / radiusFromStar;
	m_orbit = Orbit(pos, starPos, orbitSpeed);

	m_shaderRandomSeed = Random::randFloat(0.0f, 1.0f);
}

void Planet::draw(sf::RenderWindow& window, EffectsEmitter& emitter) {
	m_orbit.draw(window);
	emitter.drawGlow(window, m_shape.getPosition(), 750.0f);
	emitter.drawPlanet(window, m_shape, m_shaderRandomSeed);
}

void Planet::update() {
	m_shape.setPosition(m_orbit.update());
}