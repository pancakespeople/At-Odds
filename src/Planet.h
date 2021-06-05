#pragma once

#include <SFML/Graphics.hpp>
#include "EffectsEmitter.h"
#include "Orbit.h"
#include "ParticleSystem.h"

class Planet {
public:
	Planet(sf::Vector2f pos, sf::Vector2f starPos);

	void draw(sf::RenderWindow& window, EffectsEmitter& emitter);

	void update();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_shape;
		archive & m_shaderRandomSeed;
		archive & m_orbit;
	}
	
	Planet() {}

	sf::CircleShape m_shape;
	float m_shaderRandomSeed = 1.0f;
	Orbit m_orbit;
};
