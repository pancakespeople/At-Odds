#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

#include "Unit.h"

class Star;
class Spaceship;
class Faction;
class Player;
class EffectsEmitter;

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

	Building(const std::string& type, Star* star, sf::Vector2f pos, Faction* faction, bool built = true);
	Building() {}

	void draw(sf::RenderWindow& window, EffectsEmitter& emitter);
	void update(Star* currentStar);
	void construct(const Spaceship* constructor);
	void reinitAfterLoad(Star* star);
	void setPos(sf::Vector2f pos) { m_sprite.setPosition(pos); }

	static bool checkBuildCondition(const std::string& type, sf::Vector2f pos, float radius, Star* star, int allegiance, bool player = false);

	bool isBuilt() { return m_constructionPercent >= 100.0f; }

	std::string getName();
	std::string getType() { return m_type; }
	std::string getInfoString();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Unit>(*this);
		archive & m_sprite;
		archive & m_attackTargetID;
		archive & m_constructionPercent;
		archive & m_constructionSpeedMultiplier;
		archive & m_type;
	}

	void attackEnemies();

	sf::Sprite m_sprite;
	
	Unit* m_attackTarget = nullptr;
	uint32_t m_attackTargetID = 0;

	float m_constructionPercent = 0.0f;
	float m_constructionSpeedMultiplier = 1.0f;

	std::string m_type;
};

class BuildingPrototype {
public:
	BuildingPrototype() {}
	BuildingPrototype(const std::string& type);
	
	const sf::Texture* getTexture() { return m_sprite.getTexture(); }

	std::string getType() const { return m_type; }

	void draw(sf::RenderWindow& window, Star* currentStar, const Player& player);
	void setPos(const sf::Vector2f& pos) { m_sprite.setPosition(pos); }

	static bool meetsDisplayRequirements(const std::string& type, Faction* faction);

	float getRadius() { return m_radius; }
	
	sf::Vector2f getPos() { return m_sprite.getPosition(); }

private:
	std::string m_type;
	sf::Sprite m_sprite;
	float m_radius = 0.0f;
};

