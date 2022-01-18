#pragma once

#include <SFML/Graphics.hpp>
#include "TOMLCache.h"
#include "Weapon.h"
#include "TradeGoods.h"

class Star;
class Faction;
class Planet;
class ColonyBuilding;

class Colony {
public:
	const static int growthTicks = 1000;
	const static int maxPopulation = 1000000000; // 1 billion

	void update(Star* currentStar, Faction* faction, Planet* planet);

	int getPopulation() const { return m_population; }
	int getAllegiance() const { return m_allegiance; }

	float getGrowthRate(float planetHabitability) const;
	float getBuildingEffects(const std::string& effect) const;
	float getWealth() const { return m_wealth * m_stability; }
	float getStability() const { return m_stability; }
	float getResourceExploitation(const std::string& type, const Planet& planet) const;

	bool isColonizationLegal(int allegiance);
	bool hasBuildingOfType(const std::string& string);
	bool hasBuildingFlag(const std::string& flag);
	bool hasUndiscoveredResources(Planet& planet);

	// Buys a building for a faction, returns false if failed
	bool buyBuilding(const ColonyBuilding& building, Faction* faction, Planet& planet);
	
	void setFactionColonyLegality(int allegiance, bool legality);
	void setTicksToNextBus(int ticks) { m_ticksToNextBus = ticks; }
	void addPopulation(int pop);
	void subtractPopulation(int pop);
	void setAllegiance(int id) { m_allegiance = id; }
	void setFactionColor(sf::Color color) { m_factionColor = color; }
	void addBuilding(const ColonyBuilding& building) { m_buildings.push_back(building); }
	void onBuildingBuild(ColonyBuilding& building);
	void exploration(Planet* planet, Faction* faction);
	void addWealth(float wealth) { m_wealth += wealth; }
	void removeWealth(float wealth);
	void addStability(float stab);
	void removeStability(float stab);
	void onColonization(Planet& planet);

	sf::Color getFactionColor() { return m_factionColor; }

	std::vector<ColonyBuilding>& getBuildings() { return m_buildings; }
	ColonyBuilding* getBuildingOfType(const std::string& type);

	TradeGoods& getTradeGoods() { return m_tradeGoods; }

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
		archive & m_defenseCannon;
		archive & m_defenseCannonEnabled;
		archive & m_explorationEnabled;
		archive & m_explorationEventTimer;
		archive & m_tradeGoods;
		archive & m_stability;
		archive & m_wealth;
		archive & m_newBuildingNames;
		archive & m_revealResourceTimer;
	}

	int m_population = 0;
	int m_allegiance = -1;
	int m_ticksUntilNextGrowth = growthTicks;
	int m_ticksToNextBus = 500;
	int m_ticksToNextResourceExploit = 1000;
	int m_explorationEventTimer = 0;
	int m_revealResourceTimer = 0;

	float m_wealth = 0.0f;
	float m_stability = 1.0f;

	bool m_defenseCannonEnabled = false;
	bool m_explorationEnabled = false;

	sf::Color m_factionColor = sf::Color(175, 175, 175);

	std::unordered_map<int, bool> m_factionColonyLegality;
	std::vector<ColonyBuilding> m_buildings;
	std::deque<std::string> m_newBuildingNames;

	Weapon m_defenseCannon = Weapon("FLAK_CANNON");
	TradeGoods m_tradeGoods;
};

class ColonyBuilding {
public:
	ColonyBuilding(const std::string& type);

	void build(Colony& colony);

	std::string getName() const;
	std::string getDescription() const;
	std::string getType() const { return m_type; }
	std::string getEffectsString() const;
	std::unordered_map<std::string, float> getResourceCost(Planet& planet) const;
	std::vector<std::string> getFlags() const;

	bool isBuilt() const { return m_percentBuilt >= 100.0f; }
	bool hasFlag(const std::string& flag) const;

	float getPercentBuilt() const { return m_percentBuilt; }

	template<typename T>
	T getEffect(const std::string& val, T defaultVal) const {
		const toml::table& table = TOMLCache::getTable("data/objects/colonybuildings.toml");

		return table[m_type][val].value_or(defaultVal);
	}

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