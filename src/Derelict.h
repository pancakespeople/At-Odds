#pragma once
#include <SFML/Graphics.hpp>

#include "Collider.h"

class Derelict {
public:
	Derelict(sf::Vector2f pos);
	
	void draw(sf::RenderWindow& window);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_sprite;
		archive & m_collider;
	}
	
	Derelict() {}

	sf::Sprite m_sprite;
	Collider m_collider;
};