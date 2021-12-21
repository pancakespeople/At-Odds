#include "gamepch.h"
#include "Colony.h"
#include "Planet.h"
#include "Star.h"
#include "Faction.h"
#include "TOMLCache.h"
#include "Util.h"
#include "Random.h"

bool Colony::isColonizationLegal(int allegiance) {
	if (m_factionColonyLegality.count(allegiance) == 0) return false;
	else return m_factionColonyLegality[allegiance];
}

void Colony::setFactionColonyLegality(int allegiance, bool legality) {
	m_factionColonyLegality[allegiance] = legality;
}

void Colony::update(Star* currentStar, Faction* faction, Planet* planet) {
	// Grow population
	if (m_ticksUntilNextGrowth == 0) {
		float growthRate = getGrowthRate(planet->getHabitability());
		float growth = m_population * growthRate;
		addPopulation(growth);
		m_ticksUntilNextGrowth = Colony::growthTicks;
	}
	else {
		m_ticksUntilNextGrowth--;
	}

	if (faction != nullptr) setFactionColor(faction->getColor());

	// Spawn space bus
	if (m_population >= 1000 && faction != nullptr) {
		if (m_ticksToNextBus == 0) {
			Star* targetStar = HabitatMod::findBusStarDestination(currentStar, faction);;

			if (targetStar->getPlanets().size() > 0) {
				Planet* targetPlanet = HabitatMod::findBusPlanetDestination(m_allegiance, targetStar, planet);

				if (targetPlanet != nullptr) {
					if (m_population < 50000) {
						planet->createSpaceBus(faction->getColor(), currentStar, targetStar, targetPlanet, "SPACE_BUS", 1000, 1000);

						m_population -= 1000;
					}
					else {
						planet->createSpaceBus(faction->getColor(), currentStar, targetStar, targetPlanet, "SPACE_TRAIN", 10000, 10000);

						m_population -= 10000;
					}
				}
			}
			
			float busSpawnTimeMultiplier = getBuildingEffects("busSpawnTimeMultiplier");
			if (m_population < 50000) {
				m_ticksToNextBus = HabitatMod::calcBusTickTimer(m_population) * busSpawnTimeMultiplier;
			}
			else {
				m_ticksToNextBus = std::max(HabitatMod::calcBusTickTimer(m_population / 32.0f) * busSpawnTimeMultiplier, 1000.0f);
			}
		}
		else {
			m_ticksToNextBus--;
		}
	}

	// Resource exploitation
	if (faction != nullptr) {
		if (m_ticksToNextResourceExploit == 0) {
			for (Resource& resource : planet->getResources()) {
				float amount = getResourceExploitation(resource.type, *planet);
				faction->addResource(resource.type, amount);
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
}

void Colony::addPopulation(int pop) {
	// Make sure it doesn't go past the population limit
	if (m_population - maxPopulation + pop > 0) {
		return;
	}
	else {
		m_population += pop;
	}
}

bool Colony::hasBuildingOfType(const std::string& string) {
	for (ColonyBuilding& building : m_buildings) {
		if (building.getType() == string) {
			return true;
		}
	}
	return false;
}

float Colony::getGrowthRate(float planetHabitability) const {
	// Apply building modifiers
	for (const ColonyBuilding& building : m_buildings) {
		if (building.isBuilt()) {
			planetHabitability *= building.getEffect("habitabilityModifier", 1.0f);
		}
	}
	
	// Food and water shortages affect habitability
	float food = m_tradeGoods.getSupply("FOOD") == 0.0f ? 0.5f : 1.0f;
	float water = m_tradeGoods.getSupply("WATER") == 0.0f ? 0.5f : 1.0f;
	planetHabitability *= food * water;

	// Stability effect
	planetHabitability *= m_stability;

	// Negative growth rate if habitability is less than 1.0
	float growthRate = (planetHabitability - 1.0f) / 10.0f;

	return growthRate;
}

bool Colony::hasBuildingFlag(const std::string& flag) {
	for (ColonyBuilding& building : m_buildings) {
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

void Colony::subtractPopulation(int pop) {
	if (m_population - pop <= 0) {
		m_population = 0;
		m_allegiance = -1;
		m_factionColor = Faction::neutralColor;
	}
	else {
		m_population -= pop;
	}
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
		m_percentBuilt += 0.01;
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

float Colony::getResourceExploitation(const std::string& type, const Planet& planet) const {
	float multiplier = 1.0f;

	for (const ColonyBuilding& building : m_buildings) {
		if (building.isBuilt()) {
			multiplier *= building.getEffect("exploitationModifier", 1.0f);
		}
	}

	return (m_population * planet.getResourceAbundance(type) / 250.0f) * multiplier;
}
