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
#include "Identifiable.h"
#include "Quadtree.h"
#include "AsteroidBelt.h"
#include "Asteroid.h"

class Hyperlane;
class Faction;
class GameState;
class EffectsEmitter;
class Player;
class Constellation;
class AllianceList;

class Star : public Identifiable {
public:
	Star(sf::Vector2f pos);

	void draw(sf::RenderWindow& window);
	void draw(const sf::RenderWindow & window, Renderer& renderer, Constellation& constellation, Player& player);
	void drawLocalView(sf::RenderWindow& window, Renderer& renderer, Player& player, const AllianceList& alliances, float time);
	void setPos(sf::Vector2f pos);
	void setColor(sf::Color color) { m_shape.setFillColor(color); }
	void connectHyperlane(Hyperlane* lane);
	void factionTakeOwnership(Faction* faction, bool spawnClaimUnit = false);
	void setRadius(float radius) { m_shape.setRadius(radius); }
	void worldViewClicked(sf::Event ev, GameState& state);
	void setupJumpPoints();
	void addProjectile(Projectile proj);
	void addAnimation(const Animation& anim);
	void addEffectAnimation(const EffectAnimation& anim) { m_effectAnimations.push_back(anim); }
	void cleanUpAnimations();
	void update(Constellation* constellation, const Player& player, EffectsEmitter& emitter);
	void destroyAllShips();
	void clearAnimations() { m_localViewAnimations.clear(); }
	void moveShipToOtherStar(Spaceship* ship, Star* other);
	void generatePlanets();
	void setDiscovered(bool isDiscovered, int allegiance);
	void drawUndiscovered(Renderer& renderer);
	void generateDerelicts();
	void reinitAfterLoad(Constellation* constellation);
	void setName(const std::string& name) { m_name = name; }
	void generateAsteroidBelts();
	void onClick(GameState& state, sf::Vector2f releaseMouseWorldPos, sf::Vector2f pressMouseWorldPos);
	void generateAsteroids();
	
	Spaceship* createSpaceship(const std::string& type, sf::Vector2f pos, int allegiance, sf::Color color);
	Spaceship* generateRandomShip(sf::Vector2f pos, int allegiance, sf::Color color, std::vector<std::string> allowedChassis);
	Building* createBuilding(const std::string& type, sf::Vector2f pos, Faction* faction, bool built = true);

	float getRadius() const { return m_shape.getRadius(); }
	float distBetweenStar(Star& s);
	float getOuterBoundary();

	bool isStarInRadius(Star& s, float radius) const;
	bool isInShapeRadius(float x, float y) const;
	bool isLocalViewActive() const { return m_localViewActive; }
	bool containsBuildingType(const std::string& type, bool allegianceOnly = false, int allegiance = 0) const;
	bool isDiscovered(int allegiance) const { return m_factionsDiscovered.count(allegiance); }
	bool isDrawingHidden() const { return m_drawHidden; }
	bool hasHyperlaneConnectionTo(const Star* star) const;
	bool isBlackHole() const { return m_blackHole; }

	int getAllegiance() const { return m_allegiance; }
	int numAlliedShips(int allegiance) const;
	int numAlliedShips(int allegiance, const AllianceList& alliances) const;
	int numAlliedBuildings(int allegiance, const std::string& type = "") const;
	int numAlliedBuildings(int allegiance, const AllianceList& alliances, const std::string& type = "") const;
	int numAllies(int allegiance) const;
	int numAllies(int allegiance, const AllianceList& alliances) const;
	int getPlanetIndex(Planet* planet) const;

	sf::Vector2f getPos() const { return m_shape.getPosition(); }
	sf::Vector2f getCenter() const;
	sf::Vector2f getLocalViewCenter() const;
	sf::Vector2f getRandomLocalPos(float min, float max) const;
	sf::Color getColor() const { return m_shape.getFillColor(); }

	std::vector<Hyperlane*> getHyperlanes() { return m_hyperlanes; }
	std::vector<std::unique_ptr<Spaceship>>& getSpaceships() { return m_localShips; }
	std::vector<JumpPoint>& getJumpPoints() { return m_jumpPoints; }
	std::vector<Star*> getConnectedStars();
	std::vector<std::unique_ptr<Building>>& getBuildings() { return m_buildings; }
	std::vector<Planet>& getPlanets() { return m_planets; }
	std::vector<Derelict>& getDerelicts() { return m_derelicts; }
	std::vector<Spaceship*> getAllShipsOfAllegiance(int allegiance);
	std::vector<Planet*> getEnemyPlanets(int allegiance, const AllianceList& alliances);
	std::vector<Spaceship*> getEnemyCombatShips(int allegiance, const AllianceList& alliances);
	std::vector<Building*> getBuildingsOfType(const std::string& type);
	std::vector<Asteroid>& getAsteroids() { return m_asteroids; }
	
	// First = allegiance, second = num ships
	std::unordered_map<int, int> countNumFactionShips();

	std::string getName() const { return m_name; }

	Unit* getUnitByID(unsigned int id);
	Spaceship* getShipByID(unsigned int id);
	Building* getBuildingByID(unsigned int id);
	JumpPoint* getJumpPointByID(unsigned int id);
	Planet* getPlanetByID(unsigned int id);
	Hyperlane* getHyperlaneByID(uint32_t id);
	Asteroid* getAsteroidByID(uint32_t id);

	Planet& getMostHabitablePlanet();
	Planet* getMostHabitablePlanet(int allegiance);
	ParticleSystem& getParticleSystem() { return m_particleSystem; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Identifiable>(*this);
		archive & m_shape;
		archive & m_localViewRect;
		archive & m_hyperlaneIDs;
		archive & m_jumpPoints;
		archive & m_localShips;
		archive & m_projectiles;
		archive & m_buildings;
		archive & m_localViewAnimations;
		archive & m_planets;
		archive & m_localViewActive;
		archive & m_underAttack;
		archive & m_allegiance;
		archive & m_shaderRandomSeed;
		archive & m_temperature;
		archive & m_derelicts;
		archive & m_drawHidden;
		archive & m_factionsDiscovered;
		archive & m_name;
		archive & m_blackHole;
		archive & m_asteroidBelts;
		archive & m_peaceful;
		archive & m_effectAnimations;
		archive & m_asteroids;
	}
	
	void handleCollisions(const AllianceList& alliances);

	void init(const sf::Vector2f& pos);

	Star() {}

	friend void GameState::changeToLocalView(Star* star);
	friend void GameState::changeToWorldView();
	friend void GameState::switchLocalViews(Star* star);
	
	sf::CircleShape m_shape;
	sf::RectangleShape m_localViewRect;

	std::vector<Hyperlane*> m_hyperlanes;
	std::vector<uint32_t> m_hyperlaneIDs;
	std::vector<JumpPoint> m_jumpPoints;
	std::vector<std::unique_ptr<Spaceship>> m_localShips;
	std::vector<Projectile> m_projectiles;
	std::vector<std::unique_ptr<Building>> m_buildings;
	std::vector<Animation> m_localViewAnimations;	
	std::vector<EffectAnimation> m_effectAnimations;
	std::vector<Planet> m_planets;
	std::vector<Derelict> m_derelicts;
	std::vector<AsteroidBelt> m_asteroidBelts;
	std::vector<Asteroid> m_asteroids;
	std::unordered_set<int> m_factionsDiscovered = {-1};

	bool m_localViewActive = false;
	bool m_underAttack = false;
	bool m_drawHidden = true;
	bool m_blackHole = false;
	bool m_peaceful = true;

	int m_allegiance = -1;

	float m_shaderRandomSeed = 1.0f;
	float m_temperature = 5000.0f; // Kelvin

	ParticleSystem m_particleSystem;
	Quadtree m_quadtree;

	std::string m_name;
};
