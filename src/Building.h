#pragma once
#include <SFML/Graphics.hpp>

#include "Unit.h"

class Star;

class Building : public Unit {
public:
	enum class BUILDING_TYPE {
		OUTPOST
	};

	Building(BUILDING_TYPE type, Star* star, sf::Vector2f pos, int allegiance, sf::Color color);

	void draw(sf::RenderWindow& window);

	void update(Star* star);


private:
	sf::Sprite m_sprite;
};

