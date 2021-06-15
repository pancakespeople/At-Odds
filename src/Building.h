#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

#include "Unit.h"

class Star;
class Spaceship;
class Faction;
class Player;

class Building : public Unit {
public:
	enum class BUILDING_TYPE {
		NONE,
		OUTPOST,
		LASER_TURRET,
		MACHINE_GUN_TURRET,
		GAUSS_TURRET,
		SHIP_FACTORY,
		SPACE_HABITAT
	};

	static const std::unordered_map<Building::BUILDING_TYPE, std::string> texturePaths;

	Building(BUILDING_TYPE type, Star* star, sf::Vector2f pos, int allegiance, sf::Color color, bool built = true);

	void draw(sf::RenderWindow& window);
	void update(Star* currentStar);
	void construct(const Spaceship* constructor);

	static bool checkBuildCondition(BUILDING_TYPE type, const Star* star, int allegiance, bool player = false);

	bool isBuilt() { return m_constructionPercent >= 100.0f; }

	BUILDING_TYPE getType() { return m_type; }
	
	std::string getTypeString();
	std::string getInfoString();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Unit>(*this);
		archive & m_sprite;
		archive & m_attackTarget;
		archive & m_constructionPercent;
		archive & m_constructionSpeedMultiplier;
		archive & m_type;
	}

	Building() {}
	
	void attackEnemies();

	sf::Sprite m_sprite;
	Unit* m_attackTarget = nullptr;

	float m_constructionPercent = 0.0f;
	float m_constructionSpeedMultiplier = 1.0f;

	BUILDING_TYPE m_type = BUILDING_TYPE::NONE;
};

class BuildingPrototype {
public:
	BuildingPrototype() {}
	BuildingPrototype(Building::BUILDING_TYPE type);
	
	const sf::Texture* getTexture() { return m_sprite.getTexture(); }

	Building::BUILDING_TYPE getType() const { return m_type; }

	void draw(sf::RenderWindow& window, const Star* currentStar, const Player& player);
	void setPos(const sf::Vector2f& pos) { m_sprite.setPosition(pos); }

private:
	Building::BUILDING_TYPE m_type = Building::BUILDING_TYPE::OUTPOST;
	sf::Sprite m_sprite;
};

