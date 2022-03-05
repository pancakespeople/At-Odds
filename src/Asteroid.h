#pragma once
#include "Orbit.h"

class Renderer;

class Asteroid {
public:
	Asteroid(sf::Vector2f pos, sf::Vector2f starPos);

	void draw(Renderer& renderer);
	void update();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_sprite;
		archive & m_orbit;
	}

	Asteroid() = default;

	sf::Sprite m_sprite;
	Orbit m_orbit;
};