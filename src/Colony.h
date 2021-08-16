#pragma once

#include <SFML/Graphics.hpp>

class Star;
class Faction;
class Planet;

class ColonyBuilding {
public:
	ColonyBuilding(const std::string& type);

	void update();

	std::string getName() const;
	std::string getDescription() const;
	std::string getType() const { return m_type; }
	std::string getEffectsString() const;
	std::unordered_map<std::string, float> getResourceCost(Planet& planet) const;

	bool isBuilt() { return m_percentBuilt >= 100.0f; }
	bool hasFlag(const std::string& flag);

	float getHabitabilityModifier() const;
	float getExploitationModifer() const;
	float getPercentBuilt() const { return m_percentBuilt; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_type;
		archive & m_percentBuilt;
	}
	
	ColonyBuilding() {}

	std::string m_type;
	float m_percentBuilt = 0.0f;
};

class Colony {
public:
	const static int growthTicks = 1000;
	const static int maxPopulation = 1000000000; // 1 billion

	void update(Star* currentStar, Faction* faction, Planet* planet);

	int getPopulation() { return m_population; }
	int getAllegiance() { return m_allegiance; }

	float getGrowthRate(float planetHabitability);
	
	bool isColonizationLegal(int allegiance);
	bool hasBuildingOfType(const std::string& string);
	bool hasBuildingFlag(const std::string& flag);

	void setFactionColonyLegality(int allegiance, bool legality);
	void setTicksToNextBus(int ticks) { m_ticksToNextBus = ticks; }
	void addPopulation(int pop);
	void setAllegiance(int id) { m_allegiance = id; }
	void setFactionColor(sf::Color color) { m_factionColor = color; }
	void addBuilding(const ColonyBuilding& building) { m_buildings.push_back(building); }

	sf::Color getFactionColor() { return m_factionColor; }

	std::vector<ColonyBuilding>& getBuildings() { return m_buildings; }
	ColonyBuilding* getBuildingOfType(const std::string& type);

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
		archive & m_buildings;
	}

	int m_population = 0;
	int m_allegiance = -1;
	int m_ticksUntilNextGrowth = growthTicks;
	int m_ticksToNextBus = 500;
	int m_ticksToNextResourceExploit = 1000;

	sf::Color m_factionColor = sf::Color(175, 175, 175);

	std::unordered_map<int, bool> m_factionColonyLegality;
	std::vector<ColonyBuilding> m_buildings;
};