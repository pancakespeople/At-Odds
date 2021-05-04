#pragma once
#include <SFML/Graphics.hpp>

#include "Unit.h"

class Star;
class Spaceship;

class Building : public Unit {
public:
	enum class BUILDING_TYPE {
		OUTPOST
	};

	Building(BUILDING_TYPE type, Star* star, sf::Vector2f pos, int allegiance, sf::Color color);

	void draw(sf::RenderWindow& window);

	void update();


private:
	void attackEnemies();

	sf::Sprite m_sprite;

	Unit* m_attackTarget = nullptr;
};

