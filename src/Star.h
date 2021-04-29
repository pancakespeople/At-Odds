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

class Star {
public:
	Star(sf::Vector2f pos);

	Star();

	void draw(sf::RenderWindow& window);

	void draw(sf::RenderWindow& window, sf::Shader& shader);

	void drawLocalView(sf::RenderWindow& window, EffectsEmitter& emitter, Player& player, float time);

	void setPos(sf::Vector2f pos);

	sf::Vector2f getPos() { return m_shape.getPosition(); }

	void setColor(sf::Color color) { m_shape.setFillColor(color); }

	float getRadius() { return m_shape.getRadius(); }

	float distBetweenStar(Star& s);

	bool isStarInRadius(Star& s, float radius);

	void connectHyperlane(Hyperlane* lane);

	std::vector<Hyperlane*> getHyperlanes() { return m_hyperlanes; }

	void factionTakeOwnership(Faction* faction, bool spawnClaimUnit = false);

	void setRadius(float radius) { m_shape.setRadius(radius); }

	bool isInShapeRadius(float x, float y);

	sf::Vector2f getCenter();

	void clicked(sf::Event ev, GameState& state);

	void setupJumpPoints();

	sf::Vector2f getLocalViewCenter();

	void addSpaceship(Spaceship* ship);
	
	void removeSpaceship(Spaceship* ship);
	
	std::vector<Spaceship*>& getSpaceships() { return m_localShips; }

	std::vector<JumpPoint>& getJumpPoints() { return m_jumpPoints; }

	void addProjectile(Projectile proj);

	void addAnimation(Animation&& anim);
	
	void cleanUpAnimations();

	void update();

	bool isLocalViewActive() { return m_localViewActive; }

	int getAllegiance() { return m_allegiance; }

	void destroyAllShips();

	std::vector<Star*> getConnectedStars();

	int numAlliedShips(int allegiance);

	void clearAnimations() { m_localViewAnimations.clear(); }

	sf::Vector2f getRandomLocalPos(float min, float max);

	void createBuilding(Building building) { m_buildings.push_back(building); }

private:
	void handleCollisions();

	friend void GameState::changeToLocalView(Star* star);

	friend void GameState::changeToWorldView();
	
	sf::CircleShape m_shape;
	sf::Sprite m_localViewSprite;

	std::vector<Hyperlane*> m_hyperlanes;
	std::vector<JumpPoint> m_jumpPoints;
	std::vector<Spaceship*> m_localShips;
	std::vector<Projectile> m_projectiles;
	std::vector<Building> m_buildings;
	
	std::vector<Animation> m_localViewAnimations;

	bool m_localViewActive = false;
	bool m_multipleFactionsPresent = false;

	int m_allegiance = -1;
};

