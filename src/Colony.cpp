#include "gamepch.h"
#include "Colony.h"
#include "Planet.h"
#include "Star.h"
#include "Faction.h"
#include "TOMLCache.h"
#include "Util.h"
#include "Random.h"
#include "Math.h"

bool Colony::isColonizationLegal(int allegiance) const {
	if (m_factionColonyLegality.count(allegiance) == 0) return false;
	else return m_factionColonyLegality.at(allegiance);
}

void Colony::setFactionColonyLegality(int allegiance, bool legality) {
	m_factionColonyLegality[allegiance] = legality;
}

void Colony::update(Star* currentStar, Faction* faction, Planet* planet) {
	// Grow population
	if (m_ticksToNextGridUpdate == 0) {
		updateGrid(*planet);
		
		m_ticksToNextGridUpdate = GRID_UPDATE_TICKS;
	}
	else {
		m_ticksToNextGridUpdate--;
	}

	if (faction != nullptr) setFactionColor(faction->getColor());

	// Spawn space bus
	if (m_ticksToNextBus == 0) {
		sf::Vector2i mostPopulatedTile;
		int mostPop = 0;

		if (isGridGenerated()) {
			mostPopulatedTile = planet->getColony().getMostPopulatedTile();
			mostPop = getTile(mostPopulatedTile).population;
		}

		if (mostPop >= 5000 && faction != nullptr) {
			Star* targetStar = HabitatMod::findBusStarDestination(currentStar, faction);;

			if (targetStar->getPlanets().size() > 0) {
				Planet* targetPlanet = HabitatMod::findBusPlanetDestination(m_allegiance, targetStar, planet);

				if (targetPlanet != nullptr) {

					if (mostPop < 100000) {
						planet->createSpaceBus(faction->getColor(), currentStar, targetStar, targetPlanet, "SPACE_BUS", 1000, 1000);

						changePopulation(-1000, mostPopulatedTile);
					}
					else {
						planet->createSpaceBus(faction->getColor(), currentStar, targetStar, targetPlanet, "SPACE_TRAIN", 10000, 10000);

						changePopulation(-10000, mostPopulatedTile);
					}
				}
			}
		}
		
		float busSpawnTimeMultiplier = getBuildingEffects("busSpawnTimeMultiplier");
		if (mostPop < 50000) {
			m_ticksToNextBus = HabitatMod::calcBusTickTimer(mostPop) * busSpawnTimeMultiplier;
		}
		else {
			m_ticksToNextBus = std::max(HabitatMod::calcBusTickTimer(mostPop / 32.0f) * busSpawnTimeMultiplier, 1000.0f);
		}
	}
	else {
		m_ticksToNextBus--;
	}


	// Resource exploitation
	if (faction != nullptr) {
		if (m_ticksToNextResourceExploit == 0) {
			if (hasBuildingOfType("BASIC_MINING", true)) {
				for (Resource& resource : planet->getResources()) {
					if (!resource.hidden) {
						float amount = getResourceExploitation(resource, *planet);
						faction->addResource(resource.type, amount);
					}
				}
			}
			m_ticksToNextResourceExploit = 1000;
		}
		else {
			m_ticksToNextResourceExploit--;
		}
	}

	// Build colony buildings
	for (ColonyBuilding& building : m_buildings) {
		if (!building.isBuilt()) {
			building.build(*this);
			break; // Only build buildings one at a time
		}
	}

	// Orbital defense
	if (faction != nullptr) {
		if (m_defenseCannonEnabled) {
			m_defenseCannon.update();
			if (!m_defenseCannon.isOnCooldown()) {
				m_defenseCannon.fireAtNearestEnemyCombatShip(planet->getPos(), faction->getID(), currentStar);
			}
		}
	}

	// Exploration
	if (m_explorationEnabled && faction != nullptr) exploration(planet, faction);

	// Trade goods
	m_tradeGoods.update(currentStar, faction, planet);

	// News events
	while (m_newBuildingNames.size() != 0) {
		if (faction != nullptr) {
			faction->addNewsEvent("The construction of the " + m_newBuildingNames.front() + " on " + planet->getName(currentStar) + " has completed.");
		}
		m_newBuildingNames.pop_front();
	}

	// Reveal resources
	if (faction != nullptr) {
		if (hasUndiscoveredResources(*planet)) {
			if (m_revealResourceTimer == 0) {
				std::vector<int> undiscoveredIndices;
				std::vector<Resource>& resources = planet->getResources();
				for (int i = 0; i < resources.size(); i++) {
					if (resources[i].hidden) {
						undiscoveredIndices.push_back(i);
					}
				}

				int rnd = Random::randInt(0, undiscoveredIndices.size() - 1);
				Resource& resource = resources[undiscoveredIndices[rnd]];
				resource.hidden = false;
				m_revealResourceTimer = Random::randInt(300, 12500);

				faction->addNewsEvent("We have discovered " + resource.getName() + " on " + planet->getName(currentStar) + ".", sf::Color::Green);
			}
			else {
				m_revealResourceTimer--;
			}
		}
	}

	// Expeditions
	for (int i = 0; i < m_expeditions.size(); i++) {
		if (m_expeditions[i].finishTimer == 0) {
			Tile& tile = getTile(m_expeditions[i].tileDestination);
			changePopulation(m_expeditions[i].population, m_expeditions[i].tileDestination);

			tile.anomaly = false;

			switch (tile.tileFlag) {
				case Tile::TileFlag::COMMON_ORE:
					planet->discoverResource("COMMON_ORE", faction, *currentStar);
					break;
				case Tile::TileFlag::UNCOMMON_ORE:
					planet->discoverResource("UNCOMMON_ORE", faction, *currentStar);
					break;
				case Tile::TileFlag::RARE_ORE:
					planet->discoverResource("RARE_ORE", faction, *currentStar);
					break;
			}

			m_expeditions.erase(m_expeditions.begin() + i);
			i--;
		}
		else {
			m_expeditions[i].finishTimer--;
		}
	}
}

bool Colony::hasBuildingOfType(const std::string& string, bool builtOnly) const {
	for (const ColonyBuilding& building : m_buildings) {
		if (building.getType() == string) {
			if (builtOnly) {
				return building.isBuilt();
			}
			return true;
		}
	}

	return false;
}

float Colony::getGrowthRate(float planetHabitability, std::string* outInfoString) const {
	float originalHabitability = planetHabitability;

	// Apply building modifiers
	float buildingEffect = 0.0f;
	for (const ColonyBuilding& building : m_buildings) {
		if (building.isBuilt()) {
			float effect = building.getEffect("habitabilityModifier", 1.0f);
			planetHabitability *= effect;
			buildingEffect += effect;
		}
	}

	// Food and water shortages affect habitability
	float food = m_tradeGoods.getSupply("FOOD") == 0.0f ? 0.5f : 1.0f;
	float water = m_tradeGoods.getSupply("WATER") == 0.0f ? 0.5f : 1.0f;
	planetHabitability *= food * water;

	// Stability effect
	planetHabitability *= m_stability;

	// Overpopulation effect
	float overpopulation = std::max(getPopulation() - getPopulationLimit(), 0.0f);
	float overpopMult = 1.0f - (overpopulation / 10000.0f);
	planetHabitability *= overpopMult;

	// Negative growth rate if habitability is less than 1.0
	float habitabilityEffect = (planetHabitability - 1.0f) / 10.0f;
	float growthRate = habitabilityEffect;

	// Fill out info string
	if (outInfoString != nullptr) {
		outInfoString->clear();

		outInfoString->append("Planet habitability: " + Util::percentify(originalHabitability) + "\n");

		if (buildingEffect != 0.0f) {
			outInfoString->append("Buildings: " + Util::percentify(buildingEffect) + "\n");
		}

		if (food * water != 1.0f) {
			outInfoString->append("Food and/or water shortage: " + Util::percentify(food * water) + "\n");
		}

		if (m_stability != 1.0f) {
			outInfoString->append("Stability: " + Util::percentify(m_stability) + "\n");
		}

		if (overpopMult != 1.0f) {
			outInfoString->append("Overpopulation: " + Util::percentify(overpopMult) + "\n");
		}
	}

	return growthRate;
}

bool Colony::hasBuildingFlag(const std::string& flag) const {
	for (const ColonyBuilding& building : m_buildings) {
		if (building.hasFlag(flag) && building.isBuilt()) {
			return true;
		}
	}
	return false;
}

bool Colony::buyBuilding(const ColonyBuilding& building, Faction* faction, Planet& planet) {
	if (!hasBuildingOfType(building.getType())) {
		if (faction->canSubtractResources(building.getResourceCost(planet))) {
			faction->subtractResources(building.getResourceCost(planet));
			planet.getColony().addBuilding(building);
			return true;
		}
	}
	return false;
}

ColonyBuilding* Colony::getBuildingOfType(const std::string& type) {
	for (ColonyBuilding& building : m_buildings) {
		if (building.getType() == type) {
			return &building;
		}
	}
	return nullptr;
}

float Colony::getBuildingEffects(const std::string& effect) const {
	float mult = 1.0f;
	for (const ColonyBuilding& building : m_buildings) {
		if (building.isBuilt()) mult *= building.getEffect(effect, 1.0f);
	}
	return mult;
}

void Colony::onBuildingBuild(ColonyBuilding& building) {
	if (hasBuildingFlag("ENABLE_ORBITAL_CANNON")) {
		m_defenseCannonEnabled = true;
	}

	if (hasBuildingFlag("ENABLE_EXPLORATION")) {
		m_explorationEnabled = true;
		m_explorationEventTimer = Random::randInt(10000, 20000);
	}

	m_newBuildingNames.push_back(building.getName());
}

void Colony::exploration(Planet* planet, Faction* faction) {
	if (m_explorationEventTimer == 0) {
		float rnd = Random::randFloat(0.0f, 1.0f);

		if (rnd < 0.75f) {
			// Boring exploration event

			// Gain resources
			faction->addResource("COMMON_ORE", Random::randFloat(0.0f, 25.0f));
			faction->addResource("UNCOMMON_ORE", Random::randFloat(0.0f, 25.0f));
			faction->addResource("RARE_ORE", Random::randFloat(0.0f, 25.0f));
			planet->addEvent("EXPLORATION_RESOURCES");
			faction->addAnnouncementEvent("Explorers on one of your planets found some resources");
		}
		else if (rnd < 0.9f) {
			// Interesting exploration event

			// Found weapon
			DesignerWeapon weapon = faction->addRandomWeapon();
			planet->addEvent("EXPLORATION_WEAPON");
			faction->addAnnouncementEvent("Your explorers found a " + weapon.name + " weapon design");
		}
		else {
			// Exciting exploration event
		}

		m_explorationEventTimer = Random::randInt(10000, 20000);
	}
	else {
		m_explorationEventTimer--;
	}
}

void Colony::addStability(float stab) {
	if (m_stability + stab > 1.0f) m_stability = 1.0f;
	else m_stability += stab;
}

void Colony::removeStability(float stab) {
	if (m_stability - stab < 0.0f) m_stability = 0.0f;
	else m_stability -= stab;
}

void Colony::removeWealth(float wealth) {
	if (m_wealth - wealth < 0.0f) m_wealth = 0.0f;
	else m_wealth -= wealth;
}

float Colony::getPopulationLimit() const {
	float limit = 10000.0f; // Population limit with no modifiers
	for (auto& building : m_buildings) {
		if (building.isBuilt()) {
			limit += building.getEffect("addsPopulationLimit", 0.0f);
		}
	}
	return limit;
}

ColonyBuilding::ColonyBuilding(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/colonybuildings.toml");

	assert(table.contains(type));

	m_type = type;
}

std::string ColonyBuilding::getName() const {
	const toml::table& table = TOMLCache::getTable("data/objects/colonybuildings.toml");
	return table[m_type]["name"].value_or("Unnamed");
}

std::string ColonyBuilding::getDescription() const {
	const toml::table& table = TOMLCache::getTable("data/objects/colonybuildings.toml");
	return table[m_type]["description"].value_or("");
}

std::unordered_map<std::string, float> ColonyBuilding::getResourceCost(Planet& planet) const {
	std::unordered_map<std::string, float> cost;
	const toml::table& table = TOMLCache::getTable("data/objects/colonybuildings.toml");

	if (table[m_type].as_table()->contains("baseCost")) {
		for (int i = 0; i < table[m_type]["baseCost"].as_array()->size(); i++) {
			std::string resourceType = table[m_type]["baseCost"][i][0].value_or("");
			cost[resourceType] = table[m_type]["baseCost"][i][1].value_or(0.0f);

			// Lower habitability planets have higher costs
			float mod = 1.0f / planet.getHabitability();
			cost[resourceType] *= mod;
		}
	}

	return cost;
}

void ColonyBuilding::build(Colony& colony) {
	if (!isBuilt()) {
		m_percentBuilt += getEffect("buildProgress", 0.01f);
		if (isBuilt()) colony.onBuildingBuild(*this);
	}
}

std::string ColonyBuilding::getEffectsString() const {
	std::stringstream effects;
	effects << std::fixed << std::setprecision(1);

	const toml::table& table = TOMLCache::getTable("data/objects/colonybuildingeffectstext.toml");

	for (auto& item : *table["MODIFIERS"].as_table()) {
		float modifier = getEffect(item.first, 1.0f);
		if (modifier != 1.0f) {
			effects << item.second.value_or("") << ": " << Util::percentify(modifier, 1) << "\n";
		}
	}

	for (auto& item : *table["FLAGS"].as_table()) {
		if (hasFlag(item.first)) {
			effects << item.second.value_or("") << "\n";
		}
	}

	for (auto& item : *table["ADDERS"].as_table()) {
		float adder = getEffect(item.first, 0.0f);
		if (adder != 0.0f) {
			effects << item.second.value_or("") << ": " << Util::cutOffDecimal(adder, 2) << "\n";
		}
	}

	return effects.str();
}

bool ColonyBuilding::hasFlag(const std::string& flag) const {
	const auto& table = TOMLCache::getTable("data/objects/colonybuildings.toml");

	auto* arr = table[m_type]["flags"].as_array();
	if (arr != nullptr) {
		for (auto& elem : *arr) {
			if (elem.value_or("") == flag) {
				return true;
			}
		}
	}
	return false;
}

std::vector<std::string> ColonyBuilding::getFlags() const {
	const auto& table = TOMLCache::getTable("data/objects/colonybuildings.toml");
	std::vector<std::string> flags;

	auto* arr = table[m_type]["flags"].as_array();
	if (arr != nullptr) {
		for (auto& elem : *arr) {
			flags.push_back(elem.value_or(""));
		}
	}

	return flags;
}

float Colony::getResourceExploitation(const Resource& resource, const Planet& planet) const {
	if (resource.hidden) {
		return 0.0f;
	}

	float multiplier = 1.0f;

	for (const ColonyBuilding& building : m_buildings) {
		if (building.isBuilt()) {
			multiplier *= building.getEffect("exploitationModifier", 1.0f);
		}
	}

	return (getPopulation() * planet.getResourceAbundance(resource.type) / 250.0f) * multiplier;
}

void Colony::onColonization(Planet& planet) {
	if (hasUndiscoveredResources(planet)) {
		m_revealResourceTimer = Random::randInt(300, 12500);
	}
	else {
		m_revealResourceTimer = 0;
	}

	if (m_tiles.size() == 0) {
		// Generate gridpoints
		for (int i = 0; i < GRID_SIZE; i++) {
			m_tiles.push_back(Tile{});
		}

		generateAnomalies();
	}

	m_ticksToNextGridUpdate = GRID_UPDATE_TICKS;
}

bool Colony::hasUndiscoveredResources(const Planet& planet) const {
	for (const Resource& resource : planet.getResources()) {
		if (resource.hidden) {
			return true;
		}
	}
	return false;
}

bool ColonyBuilding::isBuildable(const Colony& colony) const {
	const auto& table = TOMLCache::getTable("data/objects/colonybuildings.toml");
	
	// Check required buildings
	bool hasRequiredBuildings = true;
	if (table[m_type].as_table()->contains("requiresBuildings")) {
		for (auto& arr : *table[m_type]["requiresBuildings"].as_array()) {
			hasRequiredBuildings = colony.hasBuildingOfType(arr.value_or(""), true);
		}
	}

	return hasRequiredBuildings;
}

int Colony::getPopulation() const {
	int population = 0;
	for (const Tile& point : m_tiles) {
		population += point.population;
	}
	return population;
}

Colony::Tile& Colony::getTile(sf::Vector2i point) {
	assert(point.x >= 0 && point.x < GRID_LENGTH &&
		point.y >= 0 && point.y < GRID_LENGTH);
	return m_tiles[point.x + point.y * GRID_LENGTH];
}

std::vector<sf::Vector2i> Colony::getPopulatedTiles(int minPopulation) const {
	std::vector<sf::Vector2i> points;
	for (int i = 0; i < m_tiles.size(); i++) {
		if (m_tiles[i].population > minPopulation) {
			int x = i % GRID_LENGTH;
			int y = i / GRID_LENGTH;
			points.push_back({ x, y });
		}
	}
	return points;
}

void Colony::changePopulation(int pop, sf::Vector2i tile) {
	changePopulation(pop, getTile(tile));
}

void Colony::changePopulation(int pop, Tile& tile) {
	if (pop > 0) {
		if (getPopulation() - MAX_POPULATION + pop > 0) {
			return;
		}
		else {
			tile.population += pop;
		}
	}
	else {
		if (tile.population + pop < 0) {
			tile.population = 0;
		}
		else {
			tile.population += pop;
		}
	}
}

void Colony::changeWorldPopulation(int pop) {
	auto populatedTiles = getPopulatedTiles();

	int maxTilePop = MAX_POPULATION / GRID_SIZE;
	for (auto tile : populatedTiles) {
		int population = getTile(tile).population;
		if (population + pop < maxTilePop && population + pop > 0) {
			getTile(tile).population += pop;
		}
	}
}

sf::Vector2i Colony::getMostPopulatedTile() const {
	const Tile* mostPopulated = &m_tiles[0];
	sf::Vector2i mostPopulatedCoords = { 0, 0 };
	for (int i = 0; i < m_tiles.size(); i++) {
		if (m_tiles[i].population > mostPopulated->population) {
			mostPopulated = &m_tiles[i];

			int x = i % GRID_LENGTH;
			int y = i / GRID_LENGTH;
			mostPopulatedCoords = { x, y };
		}
	}
	return mostPopulatedCoords;
}

sf::Vector2i Colony::getRandomTile() const {
	int x = Random::randInt(0, GRID_LENGTH - 1);
	int y = Random::randInt(0, GRID_LENGTH - 1);
	return { x, y };
}

void Colony::updateGrid(Planet& planet) {
	int population = getPopulation();

	if (population > 0) {
		// Grow population
		
		float growthRate = getGrowthRate(planet.getHabitability());
		for (Tile& tile : m_tiles) {
			changePopulation(tile.population * growthRate, tile);
		}

		// Spread population out
		auto populatedTiles = getPopulatedTiles();
		for (sf::Vector2i tilePos : populatedTiles) {
			Tile& tile = getTile(tilePos);
			auto adjacentTiles = getAdjacentTiles(tilePos);

			// 10% of population goes to adjacent tiles
			int disperseEach = tile.population * 0.1f / adjacentTiles.size();
			int toSubtract = 0;

			for (sf::Vector2i adjTilePos : adjacentTiles) {
				Tile& adjTile = getTile(adjTilePos);

				if (!adjTile.anomaly) {
					changePopulation(disperseEach, adjTile);
					toSubtract -= disperseEach;
				}
			}

			changePopulation(toSubtract, tile);
		}
	}
}

std::vector<sf::Vector2i> Colony::getAdjacentTiles(sf::Vector2i point) const {
	std::vector<sf::Vector2i> tiles;
	
	for (int y = point.y - 1; y <= point.y + 1; y++) {
		for (int x = point.x - 1; x <= point.x + 1; x++) {
			if (x >= 0 && x < GRID_LENGTH &&
				y >= 0 && y < GRID_LENGTH &&
				!(x == point.x && y == point.y)) {
				tiles.push_back({ x, y });
			}
		}
	}

	return tiles;
}

std::string Colony::getCityTexturePath(int population, int cityVariant) {
	using namespace std::literals::string_literals;

	int cityLevel = 0;
	if (population >= 1000000) {
		// 1 million+
		cityLevel = 6;
	}
	else if (population >= 100000) {
		// 100,000-1,000,000
		cityLevel = 5;
	}
	else if (population >= 10000) {
		// 10,000-100,000
		cityLevel = 4;
	}
	else if (population >= 1000) {
		// 1000-10,000
		cityLevel = 3;
	}
	else if (population >= 100) {
		// 100-1000
		cityLevel = 2;
	}
	else {
		// 1-100
		cityLevel = 1;
	}

	return "data/art/cities/"s + "city"s + std::to_string(cityLevel) + "-"s + std::to_string(cityVariant) + ".png"s;
}

Colony::Tile::Tile() {
	cityVariant = Random::randInt(1, 3);
}

void Colony::generateAnomalies() {
	for (int i = 0; i < Random::randInt(1, 3); i++) {
		Tile& tile = getTile(getRandomTile());
		float rnd = Random::randFloat(0.0f, 1.0f);

		if (rnd > 0.5f) {
			// 50% chance
			tile.tileFlag = Tile::TileFlag::COMMON_ORE;
		}
		else if (rnd > 0.15f) {
			// 35% chance
			tile.tileFlag = Tile::TileFlag::UNCOMMON_ORE;
		}
		else {
			// 15% chance
			tile.tileFlag = Tile::TileFlag::RARE_ORE;
		}

		tile.anomaly = true;
	}
}

void Colony::sendExpedition(sf::Vector2i tilePos) {
	sf::Vector2i mostPopulatedTilePos = getMostPopulatedTile();
	Tile& mostPopulatedTile = getTile(mostPopulatedTilePos);

	if (mostPopulatedTile.population > 500) {
		mostPopulatedTile.population -= 500;

		float dist = Math::distance(sf::Vector2f(mostPopulatedTilePos), sf::Vector2f(tilePos));
		m_expeditions.push_back(Expedition{ (int)dist * 2500, tilePos, 500 });
	}
}

bool Colony::hasExpeditionToTile(sf::Vector2i tilePos) const {
	for (const Expedition& ex : m_expeditions) {
		if (ex.tileDestination == tilePos) return true;
	}
	return false;
}

std::vector<sf::Vector2i> Colony::getAnomalyTiles() const {
	std::vector<sf::Vector2i> tiles;
	for (int y = 0; y < GRID_LENGTH; y++) {
		for (int x = 0; x < GRID_LENGTH; x++) {
			const Tile& tile = getTile({ x, y });
			if (tile.anomaly) tiles.push_back({ x, y });
		}
	}
	return tiles;
}