#pragma once
#include <SFML/Graphics.hpp>

#include "JumpPoint.h"
#include "Spaceship.h"
#include "Projectile.h"
#include "Animation.h"
#include "GameState.h"
#include "Building.h"
#include "ParticleSystem.h"
#include "Planet.h"
#include "Derelict.h"

class Hyperlane;
class Faction;
class GameState;
class EffectsEmitter;
class Player;
class Constellation;

class Star {
public:
	Star(sf::Vector2f pos);

	void draw(sf::RenderWindow& window);
	void draw(sf::RenderWindow& window, sf::Shader& shader);
	void drawLocalView(sf::RenderWindow& window, EffectsEmitter& emitter, Player& player, float time);
	void setPos(sf::Vector2f pos);
	void setColor(sf::Color color) { m_shape.setFillColor(color); }
	void connectHyperlane(Hyperlane* lane);
	void factionTakeOwnership(Faction* faction, bool spawnClaimUnit = false);
	void setRadius(float radius) { m_shape.setRadius(radius); }
	void clicked(sf::Event ev, GameState& state);
	void setupJumpPoints();
	void addProjectile(Projectile proj);
	void addAnimation(const Animation& anim);
	void cleanUpAnimations();
	void update(Constellation* constellation, const Player& player);
	void destroyAllShips();
	void clearAnimations() { m_localViewAnimations.clear(); }
	void moveShipToOtherStar(Spaceship* ship, Star* other);
	void generatePlanets();
	void setDiscovered(bool isDiscovered) { m_discovered = isDiscovered; }
	void drawUndiscovered(sf::RenderWindow& window, sf::Shader& shader);
	void generateDerelicts();
	
	Spaceship* createSpaceship(std::unique_ptr<Spaceship>&& ship);
	Spaceship* createSpaceship(std::unique_ptr<Spaceship>& ship);
	
	Building* createBuilding(std::unique_ptr<Building>&& building);
	Building* createBuilding(std::unique_ptr<Building>& building);

	float getRadius() const { return m_shape.getRadius(); }
	float distBetweenStar(Star& s);

	bool isStarInRadius(Star& s, float radius) const;
	bool isInShapeRadius(float x, float y) const;
	bool isLocalViewActive() const { return m_localViewActive; }
	bool containsBuildingType(Building::BUILDING_TYPE type, bool allegianceOnly = false, int allegiance = 0) const;
	bool isDiscovered() const { return m_discovered; }

	int getAllegiance() const { return m_allegiance; }
	int numAlliedShips(int allegiance) const;
	int numAlliedBuildings(int allegiance) const;
	int numAllies(int allegiance) const;

	sf::Vector2f getPos() const { return m_shape.getPosition(); }
	sf::Vector2f getCenter() const;
	sf::Vector2f getLocalViewCenter() const;
	sf::Vector2f getRandomLocalPos(float min, float max) const;

	std::vector<Hyperlane*> getHyperlanes() { return m_hyperlanes; }
	std::vector<std::unique_ptr<Spaceship>>& getSpaceships() { return m_localShips; }
	std::vector<JumpPoint>& getJumpPoints() { return m_jumpPoints; }
	std::vector<Star*> getConnectedStars();
	std::vector<std::unique_ptr<Building>>& getBuildings() { return m_buildings; }
	std::vector<Planet>& getPlanets() { return m_planets; }

	Unit* getUnitByID(unsigned int id);
	Spaceship* getShipByID(unsigned int id);
	Building* getBuildingByID(unsigned int id);
	JumpPoint* getJumpPointByID(unsigned int id);
	Planet* getPlanetByID(unsigned int id);

	Planet& getMostHabitablePlanet();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_shape;
		archive & m_localViewSprite;
		archive & m_hyperlanes;
		archive & m_jumpPoints;
		archive & m_localShips;
		archive & m_projectiles;
		archive & m_buildings;
		archive & m_localViewAnimations;
		archive & m_planets;
		archive & m_localViewActive;
		archive & m_multipleFactionsPresent;
		archive & m_allegiance;
		archive & m_shaderRandomSeed;
		archive & m_temperature;
		archive & m_discovered;
		archive & m_derelicts;
	}
	
	void handleCollisions();

	void init(const sf::Vector2f& pos);

	Star() {}

	friend void GameState::changeToLocalView(Star* star);

	friend void GameState::changeToWorldView();
	
	sf::CircleShape m_shape;
	sf::Sprite m_localViewSprite;

	std::vector<Hyperlane*> m_hyperlanes;
	std::vector<JumpPoint> m_jumpPoints;
	std::vector<std::unique_ptr<Spaceship>> m_localShips;
	std::vector<Projectile> m_projectiles;
	std::vector<std::unique_ptr<Building>> m_buildings;
	std::vector<Animation> m_localViewAnimations;	
	std::vector<Planet> m_planets;
	std::vector<Derelict> m_derelicts;

	bool m_localViewActive = false;
	bool m_multipleFactionsPresent = false;
	bool m_discovered = false;

	int m_allegiance = -1;

	float m_shaderRandomSeed = 1.0f;
	float m_temperature = 5000.0f; // Kelvin

	ParticleSystem m_particleSystem;
};

