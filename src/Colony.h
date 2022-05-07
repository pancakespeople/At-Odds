#pragma once

#include <SFML/Graphics.hpp>
#include "TOMLCache.h"
#include "Weapon.h"
#include "TradeGoods.h"

class Star;
class Faction;
class Planet;
class ColonyBuilding;
struct Resource;

class Colony {
public:
	struct Tile {
		enum class TileFlag {
			NONE,
			COMMON_ORE,
			UNCOMMON_ORE,
			RARE_ORE
		};
		
		int population = 0;
		int8_t cityVariant = 1;
		TileFlag tileFlag = TileFlag::NONE;
		bool anomaly = false;

		Tile();

	private:
		SERIALIZE {
			archive & population;
			archive & cityVariant;
			archive & tileFlag;
			archive & anomaly;
		}
	};

	struct Expedition {
		int finishTimer = 0;
		sf::Vector2i tileDestination;
		int population = 0;

	private:
		SERIALIZE {
			archive & finishTimer;
			archive & tileDestination;
			archive & population;
		}
	};
	
	const static int GRID_LENGTH = 8;
	static const int GRID_SIZE = GRID_LENGTH * GRID_LENGTH;
	const static int GRID_UPDATE_TICKS = 1000;
	const static int MAX_POPULATION = 1000000000; // 1 billion

	void update(Star* currentStar, Faction* faction, Planet* planet);

	int getPopulation() const;
	int getAllegiance() const { return m_allegiance; }

	float getGrowthRate(float planetHabitability, std::string* outInfoString = nullptr) const;
	float getBuildingEffects(const std::string& effect) const;
	float getWealth() const { return m_wealth * m_stability; }
	float getStability() const { return m_stability; }
	float getResourceExploitation(const Resource& resource, const Planet& planet) const;
	float getPopulationLimit() const;
	float getTileGrowthRate(sf::Vector2i tilePos) const;

	bool isColonizationLegal(int allegiance) const;
	bool hasBuildingOfType(const std::string& string, bool builtOnly = false) const;
	bool hasBuildingFlag(const std::string& flag) const;
	bool hasUndiscoveredResources(const Planet& planet) const;
	bool isGridGenerated() const { return m_tiles.size() > 0; }
	bool hasExpeditionToTile(sf::Vector2i tilePos) const;
	bool hasBuildingOnTile(sf::Vector2i tile) const;

	// Buys a building for a faction, returns false if failed
	bool buyBuilding(const ColonyBuilding& building, Faction* faction, Planet& planet);
	
	void setFactionColonyLegality(int allegiance, bool legality);
	void setTicksToNextBus(int ticks) { m_ticksToNextBus = ticks; }
	
	void changePopulation(int pop, sf::Vector2i tile);
	void changePopulation(int pop, Tile& tile);
	void changeWorldPopulation(int pop);
	
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
	void generateAnomalies();
	void sendExpedition(sf::Vector2i tilePos);
	void removeBuildingOnTile(sf::Vector2i tilePos);

	sf::Color getFactionColor() { return m_factionColor; }

	std::vector<ColonyBuilding>& getBuildings() { return m_buildings; }
	std::vector<sf::Vector2i> getPopulatedTiles(int minPopulation = 0) const;
	std::vector<sf::Vector2i> getAdjacentTiles(sf::Vector2i point) const;
	const std::vector<Expedition>& getExpeditions() const { return m_expeditions; }
	std::vector<sf::Vector2i> getAnomalyTiles() const;

	const std::vector<ColonyBuilding>& getBuildings() const { return m_buildings; }
	const ColonyBuilding* getBuildingAtTile(sf::Vector2i tile) const;
	ColonyBuilding* getBuildingOfType(const std::string& type);

	TradeGoods& getTradeGoods() { return m_tradeGoods; }
	sf::Vector2i getMostPopulatedTile() const;
	sf::Vector2i getRandomTile() const;

	static std::string getCityTexturePath(int population, int cityVariant);
	const Tile& getTile(sf::Vector2i point) const { return m_tiles[point.x + point.y * GRID_LENGTH]; }
	Tile& getTile(sf::Vector2i point);

private:
	SERIALIZE {
		archive & m_ticksToNextGridUpdate;
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
		archive & m_tiles;
		archive & m_expeditions;
	}

	void updateGrid(Planet& planet);

	int m_allegiance = -1;
	int m_ticksToNextGridUpdate = GRID_UPDATE_TICKS;
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
	std::vector<Tile> m_tiles;
	std::vector<Expedition> m_expeditions;

	Weapon m_defenseCannon = Weapon("FLAK_CANNON");
	TradeGoods m_tradeGoods;
};

class ColonyBuilding {
public:
	ColonyBuilding(const std::string& type);

	void build(Colony& colony);
	void setPos(sf::Vector2i pos) { m_pos = pos; }

	sf::Vector2i getPos() const { return m_pos; }

	std::string getName() const;
	std::string getDescription() const;
	std::string getType() const { return m_type; }
	std::string getEffectsString() const;
	std::string getTexturePath() const;
	std::string getExtraInfo() const;
	std::unordered_map<std::string, float> getResourceCost(Planet& planet) const;
	std::vector<std::string> getFlags() const;

	bool isBuilt() const { return m_percentBuilt >= 100.0f; }
	bool hasFlag(const std::string& flag) const;
	bool isBuildable(const Colony& colony) const;
	bool isBuildableOnTile(const Colony& colony, sf::Vector2i tilePos) const;
	bool isTileAdjacent(sf::Vector2i tile) const;
	bool isMine() const;
	bool isGlobal() const;

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
		archive & m_pos;
	}

	ColonyBuilding() {}

	std::string m_type;
	float m_percentBuilt = 0.0f;
	sf::Vector2i m_pos;
};