#pragma once

#include <SFML/Graphics.hpp>

#include "Hyperlane.h"
#include "Star.h"
#include "Spaceship.h"
#include "Faction.h"
#include "Animation.h"

class GameState;

class Constellation {
public:
	Constellation() {}

	void generateRecursiveConstellation(int sizeWidth, int sizeHeight, int numStars);
	void generateRandomHyperlanes(int size, int numStars);
	void generateRobustHyperlanes(int size, int numStars);
	void draw(sf::RenderWindow& window);
	void draw(sf::RenderWindow& window, sf::Shader& shader);
	void onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state);
	void setupStars();
	void generateFactions(int numFactions);
	void update();
	void generateNeutralSquatters();

	std::vector<std::unique_ptr<Star>>& getStars() { return m_stars; }
	std::vector<std::unique_ptr<Hyperlane>>& getHyperlanes() { return m_hyperlanes; }
	std::vector<std::unique_ptr<Spaceship>>& getShips() { return m_spaceships; }
	std::vector<Faction>& getFactions() { return m_factions; }

	Spaceship* createShipAtStar(std::unique_ptr<Spaceship>& ship);

private:
	friend class boost::serialization::access;
	
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_spaceships;
		archive & m_toBeDeletedShips;
		archive & m_stars;
		archive & m_hyperlanes;
		archive & m_factions;
	}
	
	void recursiveConstellation(std::unique_ptr<Star>& root, int numStars);
	void cleanUpDeadShips();

	float distBetweenVecs(const sf::Vector2f& a, const sf::Vector2f& b);
	float findClosestStarDistance(sf::Vector2f& targetPos);

	std::vector<std::unique_ptr<Star>> m_stars;
	std::vector<std::unique_ptr<Hyperlane>> m_hyperlanes;
	std::vector<std::unique_ptr<Spaceship>> m_spaceships;
	std::vector<Faction> m_factions;

	std::queue<std::unique_ptr<Spaceship>> m_toBeDeletedShips;
};

