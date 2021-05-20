#pragma once
#include <SFML/Graphics.hpp>

#include "JumpPoint.h"
#include "Spaceship.h"
#include "Projectile.h"
#include "Animation.h"
#include "GameState.h"
#include "Building.h"

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
	void addAnimation(Animation&& anim);
	void cleanUpAnimations();
	void update(Constellation* constellation);
	void destroyAllShips();
	void clearAnimations() { m_localViewAnimations.clear(); }
	void moveShipToOtherStar(Spaceship* ship, Star* other);
	
	Spaceship* createSpaceship(std::unique_ptr<Spaceship>& ship);
	Building* createBuilding(std::unique_ptr<Building>& building);

	float getRadius() { return m_shape.getRadius(); }
	float distBetweenStar(Star& s);

	bool isStarInRadius(Star& s, float radius);
	bool isInShapeRadius(float x, float y);
	bool isLocalViewActive() { return m_localViewActive; }

	int getAllegiance() const { return m_allegiance; }
	int numAlliedShips(int allegiance) const;
	int numAlliedBuildings(int allegiance) const;
	int numAllies(int allegiance) const;

	sf::Vector2f getPos() { return m_shape.getPosition(); }
	sf::Vector2f getCenter();
	sf::Vector2f getLocalViewCenter();
	sf::Vector2f getRandomLocalPos(float min, float max);

	std::vector<Hyperlane*> getHyperlanes() { return m_hyperlanes; }
	std::vector<std::unique_ptr<Spaceship>>& getSpaceships() { return m_localShips; }
	std::vector<JumpPoint>& getJumpPoints() { return m_jumpPoints; }
	std::vector<Star*> getConnectedStars();
	std::vector<std::unique_ptr<Building>>& getBuildings() { return m_buildings; }

	Unit* getUnitByID(unsigned int id);
	Spaceship* getShipByID(unsigned int id);
	Building* getBuildingByID(unsigned int id);
	JumpPoint* getJumpPointByID(unsigned int id);

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
		archive & m_localViewActive;
		archive & m_multipleFactionsPresent;
		archive & m_allegiance;
		archive & m_shaderRandomSeed;
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

	bool m_localViewActive = false;
	bool m_multipleFactionsPresent = false;

	int m_allegiance = -1;

	float m_shaderRandomSeed = 1.0f;
};

