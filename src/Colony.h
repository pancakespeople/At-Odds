#pragma once

#include <SFML/Graphics.hpp>

class Star;
class Faction;
class Planet;

class Colony {
public:
	const static int growthTicks = 1000;
	const static int maxPopulation = 1000000000; // 1 billion

	void update(Star* currentStar, Faction* faction, Planet* planet);

	int getPopulation() { return m_population; }
	int getAllegiance() { return m_allegiance; }

	float getGrowthRate(float planetHabitability);
	bool isColonizationLegal(int allegiance);

	void setFactionColonyLegality(int allegiance, bool legality);
	void setTicksToNextBus(int ticks) { m_ticksToNextBus = ticks; }
	void addPopulation(int pop);
	void setAllegiance(int id) { m_allegiance = id; }
	void setFactionColor(sf::Color color) { m_factionColor = color; }

	sf::Color getFactionColor() { return m_factionColor; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_population;
		archive & m_ticksUntilNextGrowth;
		archive & m_ticksToNextBus;
		archive & m_allegiance;
		archive & m_factionColor;
		archive & m_factionColonyLegality;
	}

	int m_population = 0;
	int m_allegiance = -1;
	int m_ticksUntilNextGrowth = growthTicks;
	int m_ticksToNextBus = 500;
	int m_ticksToNextResourceExploit = 1000;

	sf::Color m_factionColor = sf::Color(175, 175, 175);

	std::unordered_map<int, bool> m_factionColonyLegality;
};