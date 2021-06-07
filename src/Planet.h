#pragma once

#include <SFML/Graphics.hpp>
#include "EffectsEmitter.h"
#include "Orbit.h"
#include "ParticleSystem.h"

class Planet {
public:
	Planet(sf::Vector2f pos, sf::Vector2f starPos, float starTemperature);

	void draw(sf::RenderWindow& window, EffectsEmitter& emitter, float time);
	void update();
	void generateGasGiant();
	void generateTerrestrial(bool dwarf);

	float getTemperature() { return m_temperature; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_shape;
		archive & m_shaderRandomSeed;
		archive & m_temperature;
		archive & m_orbit;
		archive & m_gasGiant;
	}
	
	Planet() {}

	sf::CircleShape m_shape;
	
	float m_shaderRandomSeed = 1.0f;
	float m_temperature = 500.0f;
	
	bool m_gasGiant = false;

	Orbit m_orbit;
};
