#include "gamepch.h"
#include "Planet.h"
#include "Random.h"
#include "Math.h"

Planet::Planet(sf::Vector2f pos, sf::Vector2f starPos, float starTemperature) {
	m_shape.setFillColor(sf::Color(155, 155, 155));
	m_shape.setRadius(250.0f);
	m_shape.setPosition(pos);

	float radiusFromStar = Math::distance(pos, starPos);
	float orbitSpeed = 500.0f / radiusFromStar;
	float baseTemperature = std::min(starTemperature, (starTemperature * 1000.0f) / radiusFromStar);

	m_orbit = Orbit(pos, starPos, orbitSpeed);

	if (baseTemperature < 273.15f) {
		if (Random::randFloat(0.0f, 1.0f) < 0.75f) {
			// Gas giant

			generateGasGiant(baseTemperature);
		}
		else {
			// Rocky planet

			generateTerrestrial(baseTemperature);
		}
	}
	else {
		if (Random::randFloat(0.0f, 1.0f) < 0.8f) {
			// Rocky planet

			generateTerrestrial(baseTemperature);
		}
		else {
			// Gas giant

			generateGasGiant(baseTemperature);
		}
	}

	m_shaderRandomSeed = Random::randFloat(0.0f, 1.0f);

	m_shape.setOrigin(sf::Vector2f(m_shape.getRadius(), m_shape.getRadius()));
}

void Planet::draw(sf::RenderWindow& window, EffectsEmitter& emitter, float time) {
	m_orbit.draw(window);
	emitter.drawGlow(window, m_shape.getPosition(), m_shape.getRadius() * 3.0f, m_shape.getFillColor());
	emitter.drawPlanet(window, m_shape, this, m_shaderRandomSeed, time);
}

void Planet::update() {
	m_shape.setPosition(m_orbit.update());
}

void Planet::generateGasGiant(float baseTemperature) {
	m_shape.setRadius(m_shape.getRadius() * Random::randFloat(2.0f, 4.0f));
	
	if (baseTemperature < 100.0f) {
		// Blue
		int otherColors = Random::randInt(0, 155);
		m_shape.setFillColor(sf::Color(otherColors, otherColors, Random::randInt(155, 255)));

		m_type = PLANET_TYPE::ICE_GIANT;
	}
	else {
		// Orange
		int r = Random::randInt(155, 255);
		int g = r / 1.5f;
		int b = Random::randInt(0, 155);

		m_shape.setFillColor(sf::Color(r, g, b));

		m_type = PLANET_TYPE::GAS_GIANT;
	}

	m_atmosphere = Random::randFloat(1000.0f, 1000000.0f);
	m_temperature = baseTemperature;

	m_gasGiant = true;
}

void Planet::generateTerrestrial(float baseTemperature) {
	m_shape.setRadius(m_shape.getRadius() * Random::randFloat(0.75f, 1.9f));

	if (baseTemperature > 400.0f) {
		// Too hot to sustain an atmosphere

		m_atmosphere = 0.0f;
	}
	else {
		float rnd = Random::randFloat(0.0, 1.0);
		if (rnd > 0.9) {
			// 10% chance to be a toxic world

			m_atmosphere = Random::randFloat(50.0f, 200.0f);
		}
		else if (rnd > 0.45) {
			// Earth like atmosphere

			m_atmosphere = Random::randFloat(0.0f, 3.0f);
		}
		else {
			// No atmosphere

			m_atmosphere = 0.0f;
		}
	}

	m_temperature = baseTemperature + std::max(0.0f, std::log(m_atmosphere + 0.25f) * baseTemperature);

	if (m_temperature > 325.0f || m_atmosphere == 0.0f) {
		m_water = 0.0f;
	}
	else {
		m_water = Random::randFloat(0.0f, 1.0f);
	}

	// Colors
	if (m_temperature > 273.15 && m_temperature < 325.0f &&
		m_atmosphere > 0.5f && m_atmosphere < 3.0f &&
		m_water > 0.4f) {
		// Terra

		m_shape.setFillColor(sf::Color::Green);
		m_type = PLANET_TYPE::TERRA;
	}
	else {
		if (m_atmosphere >= 50.0f) {
			// Toxic

			m_shape.setFillColor(sf::Color::Yellow);
			m_type = PLANET_TYPE::TOXIC;
		}
		else if (m_water == 0.0f && m_atmosphere == 0.0f) {
			if (m_temperature > 2000.0f) {
				// Lava
				// Red

				int gb = std::min(255.0f, m_temperature * 0.05f);

				m_shape.setFillColor(sf::Color(255, 100, 100));
				m_type = PLANET_TYPE::LAVA;
			}
			else {
				// Mercury
				// Gray/white

				int rgb = Random::randInt(125, 255);

				m_shape.setFillColor(sf::Color(rgb, rgb, rgb));
				m_type = PLANET_TYPE::BARREN;
			}
		}
		else if (m_water > 0.4f && m_temperature < 273.15f) {
			// Tundra
			// White

			m_shape.setFillColor(sf::Color::White);
			m_type = PLANET_TYPE::TUNDRA;
		}
		else {
			// Desert/Martian

			if (Random::randBool()) {
				// Orange

				int r = Random::randInt(155, 255);
				int g = r / 1.5f;
				int b = Random::randInt(0, 155);

				m_shape.setFillColor(sf::Color(r, g, b));
			}
			else {
				// Sand

				int r = Random::randInt(155, 255);
				int g = std::max(0, r - 36);
				int b = std::max(0, r - 62);

				m_shape.setFillColor(sf::Color(r, g, b));
				
			}

			m_type = PLANET_TYPE::DESERT;
		}
	}
}

std::string Planet::getTypeString() const {
	switch (m_type) {
	case PLANET_TYPE::BARREN:
		return "Barren";
	case PLANET_TYPE::DESERT:
		return "Desert";
	case PLANET_TYPE::GAS_GIANT:
		return "Gas Giant";
	case PLANET_TYPE::ICE_GIANT:
		return "Ice Giant";
	case PLANET_TYPE::TERRA:
		return "Terra";
	case PLANET_TYPE::TOXIC:
		return "Toxic";
	case PLANET_TYPE::TUNDRA:
		return "Tundra";
	case PLANET_TYPE::LAVA:
		return "Lava";
	default:
		return "Unknown";
	}
}

float Planet::getHabitability() const {
	const float temperatureGoal = 288.0f;
	const float atmosGoal = 1.0f;
	const float waterGoal = 0.71f;

	float temperatureDiff = std::abs(m_temperature - temperatureGoal) / 288.0f;
	float atmosDiff = std::abs(m_atmosphere - atmosGoal);
	float waterDiff = std::abs(m_water - waterGoal);

	float diff = temperatureDiff + atmosDiff + waterDiff;
	if (diff == 0.0f) {
		return 1000000;
	}
	else {
		return 1.0f / diff;
	}
}