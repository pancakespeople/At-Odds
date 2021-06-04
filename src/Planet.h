#pragma once

#include <SFML/Graphics.hpp>
#include "EffectsEmitter.h"

class Planet {
public:
	Planet(sf::Vector2f pos);

	void draw(sf::RenderWindow& window, EffectsEmitter& emitter);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_shape;
		archive & m_shaderRandomSeed;
	}
	
	Planet() {}

	sf::CircleShape m_shape;
	float m_shaderRandomSeed = 1.0f;
};
