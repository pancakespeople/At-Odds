#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

#include "Unit.h"

class Star;
class Spaceship;

class Building : public Unit {
public:
	enum class BUILDING_TYPE {
		OUTPOST
	};

	static const std::unordered_map<Building::BUILDING_TYPE, std::string> texturePaths;

	Building(BUILDING_TYPE type, Star* star, sf::Vector2f pos, int allegiance, sf::Color color);

	void draw(sf::RenderWindow& window);

	void update();


private:
	void attackEnemies();

	sf::Sprite m_sprite;

	Unit* m_attackTarget = nullptr;
};

class BuildingPrototype {
public:
	BuildingPrototype() {}
	
	BuildingPrototype(Building::BUILDING_TYPE type);

	const sf::Texture* getTexture() { return m_sprite.getTexture(); }

private:
	Building::BUILDING_TYPE m_type = Building::BUILDING_TYPE::OUTPOST;
	sf::Sprite m_sprite;
};

