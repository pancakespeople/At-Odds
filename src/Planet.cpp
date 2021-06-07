#include "gamepch.h"
#include "Planet.h"
#include "Random.h"
#include "Math.h"

Planet::Planet(sf::Vector2f pos, sf::Vector2f starPos, float starTemperature) {
	m_shape.setFillColor(sf::Color(155, 155, 155));
	m_shape.setRadius(250.0f);
	m_shape.setPosition(pos);

	float radiusFromStar = Math::distance(pos, starPos);
	float orbitSpeed = 1000.0f / radiusFromStar;
	m_orbit = Orbit(pos, starPos, orbitSpeed);

	m_shaderRandomSeed = Random::randFloat(0.0f, 1.0f);
	m_temperature = std::min(starTemperature, (starTemperature * 1000.0f) / radiusFromStar);

	if (m_temperature < 273.15f) {
		// Mostly gas giants and frozen rocks
		if (Random::randFloat(0.0f, 1.0f) < 0.75f) {
			// Gas giant

			generateGasGiant();
		}
		else {
			// Rocky planet

			generateTerrestrial(true);
		}
	}
	else {
		// Mostly rocky planets with the occasional gas giant

		if (Random::randFloat(0.0f, 1.0f) < 0.8f) {
			// Rocky planet

			generateTerrestrial(false);
		}
		else {
			// Gas giant

			generateGasGiant();
		}
	}

	m_shape.setOrigin(sf::Vector2f(m_shape.getRadius(), m_shape.getRadius()));
}

void Planet::draw(sf::RenderWindow& window, EffectsEmitter& emitter, float time) {
	m_orbit.draw(window);
	emitter.drawGlow(window, m_shape.getPosition(), m_shape.getRadius() * 3.0f, m_shape.getFillColor());
	emitter.drawPlanet(window, m_shape, m_shaderRandomSeed, m_shape.getRadius(), m_gasGiant, time);
}

void Planet::update() {
	m_shape.setPosition(m_orbit.update());
}

void Planet::generateGasGiant() {
	m_shape.setRadius(m_shape.getRadius() * Random::randFloat(2.0f, 4.0f));
	
	if (m_temperature < 100.0f) {
		// Blue
		int otherColors = Random::randInt(0, 155);
		m_shape.setFillColor(sf::Color(otherColors, otherColors, Random::randInt(155, 255)));
	}
	else {
		// Orange
		int r = Random::randInt(155, 255);
		int g = r / 1.5f;
		int b = Random::randInt(0, 155);

		m_shape.setFillColor(sf::Color(r, g, b));
	}

	m_gasGiant = true;
}

void Planet::generateTerrestrial(bool dwarf) {
	if (dwarf) m_shape.setRadius(m_shape.getRadius() * Random::randFloat(0.25f, 0.5f));
	else m_shape.setRadius(m_shape.getRadius() * Random::randFloat(0.75f, 1.9f));

	if (Random::randBool()) {
		// Orange

		int r = Random::randInt(155, 255);
		int g = r / 1.5f;
		int b = Random::randInt(0, 155);

		m_shape.setFillColor(sf::Color(r, g, b));
	}
	else {
		// Gray/white

		int rgb = Random::randInt(125, 255);

		m_shape.setFillColor(sf::Color(rgb, rgb, rgb));
	}
}