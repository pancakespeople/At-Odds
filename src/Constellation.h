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

	std::vector<std::unique_ptr<Star>>& getStars() { return m_stars; }

	std::vector<std::unique_ptr<Hyperlane>>& getHyperlanes() { return m_hyperlanes; }

	void onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state);

	void setupStars();

	Spaceship* createShipAtStar(Spaceship ship);

	void generateFactions(int numFactions);

	std::vector<Faction>& getFactions() { return m_factions; }

	void update();

	std::vector<std::unique_ptr<Spaceship>>& getShips() { return m_spaceships; }

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

	float distBetweenVecs(const sf::Vector2f& a, const sf::Vector2f& b);

	float findClosestStarDistance(sf::Vector2f& targetPos);

	void cleanUpDeadShips();

	std::vector<std::unique_ptr<Star>> m_stars;

	std::vector<std::unique_ptr<Hyperlane>> m_hyperlanes;

	std::vector<std::unique_ptr<Spaceship>> m_spaceships;

	std::queue<std::unique_ptr<Spaceship>> m_toBeDeletedShips;

	std::vector<Faction> m_factions;
};

