#pragma once

#include <SFML/Graphics.hpp>
#include "EffectsEmitter.h"
#include "Orbit.h"
#include "ParticleSystem.h"

class Planet {
public:
	enum class PLANET_TYPE {
		UNKNOWN,
		DESERT,
		TERRA,
		BARREN,
		TUNDRA,
		GAS_GIANT,
		ICE_GIANT,
		TOXIC
	};
	
	Planet(sf::Vector2f pos, sf::Vector2f starPos, float starTemperature);

	void draw(sf::RenderWindow& window, EffectsEmitter& emitter, float time);
	void update();
	void generateGasGiant(float baseTemperature);
	void generateTerrestrial(float baseTemperature);

	float getTemperature() const { return m_temperature; }
	float getAtmosphericPressure() const { return m_atmosphere; }
	float getWater() const { return m_water; }
	float getRadius() const { return m_shape.getRadius(); }

	bool isGasGiant() const { return m_gasGiant; }

	sf::Vector2f getPos() { return m_shape.getPosition(); }

	std::string getTypeString();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_shape;
		archive & m_shaderRandomSeed;
		archive & m_temperature;
		archive & m_atmosphere;
		archive & m_gasGiant;
		archive & m_water;
		archive & m_orbit;
		archive & m_type;
	}
	
	Planet() {}

	sf::CircleShape m_shape;
	
	float m_shaderRandomSeed = 1.0f;
	float m_temperature = 500.0f; // Kelvin
	float m_atmosphere = 1.0f; // Atmospheric pressure in Earth atmospheres
	float m_water = 0.0f; // Percent of planet covered in water from 0-1
	
	bool m_gasGiant = false;

	Orbit m_orbit;
	PLANET_TYPE m_type;
};
