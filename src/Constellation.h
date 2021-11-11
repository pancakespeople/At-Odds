#pragma once

#include <SFML/Graphics.hpp>

#include "Hyperlane.h"
#include "Star.h"
#include "Spaceship.h"
#include "Faction.h"
#include "Animation.h"
#include "NameGenerator.h"

class GameState;
class Player;

class Constellation {
public:
	Constellation();

	void generateRecursiveConstellation(int sizeWidth, int sizeHeight, int numStars);
	void generateModernMegaRobustFinalConstellation(int sizeWidth, int sizeHeight, int numStars);
	void generateRandomHyperlanes(int size, int numStars);
	void generateRobustHyperlanes(int size, int numStars);
	void draw(sf::RenderWindow& window, EffectsEmitter& emitter, Player& player);
	void onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state);
	void setupStars();
	void generateFactions(int numFactions);
	void update(const Player& player);
	void generateNeutralSquatters();
	void moveShipToPurgatory(std::unique_ptr<Spaceship>& ship);
	void reinitAfterLoad();
	void onStart();

	std::vector<std::unique_ptr<Star>>& getStars() { return m_stars; }
	std::vector<std::unique_ptr<Hyperlane>>& getHyperlanes() { return m_hyperlanes; }
	std::vector<Faction>& getFactions() { return m_factions; }

	Faction* getFaction(int id);
	Star* getStarByID(uint32_t id);
	Hyperlane* getHyperlaneByID(uint32_t id);
	Spaceship* getShipByID(uint32_t id);

private:
	friend class boost::serialization::access;
	
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_stars;
		archive & m_hyperlanes;
		archive & m_factions;
		archive & m_shipPurgatory;
		archive & m_availableFactionColors;
		archive & m_border;
	}
	
	template <typename T>
	struct PurgatoryItem {
		std::unique_ptr<T> obj = nullptr;
		int timer = 1; // Ticks until destruction

		template<class Archive>
		void serialize(Archive& archive, const unsigned int version) {
			archive & obj;
			archive & timer;
		}
	};

	void recursiveConstellation(std::unique_ptr<Star>& root, int numStars);
	void cleanUpDeadShips();
	
	float distBetweenVecs(const sf::Vector2f& a, const sf::Vector2f& b);
	float findClosestStarDistance(sf::Vector2f& targetPos);

	std::vector<std::unique_ptr<Star>> m_stars;
	std::vector<std::unique_ptr<Hyperlane>> m_hyperlanes;
	std::vector<PurgatoryItem<Spaceship>> m_shipPurgatory;
	std::vector<Faction> m_factions;
	std::vector<std::pair<sf::Color, std::string>> m_availableFactionColors;
	NameGenerator m_nameGenerator;
	sf::RectangleShape m_border;
};

