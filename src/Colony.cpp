#include "gamepch.h"
#include "Colony.h"
#include "Planet.h"
#include "Star.h"
#include "Faction.h"
#include "TOMLCache.h"
#include "Util.h"

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

	// Spawn space bus
	if (m_population >= 1000 && faction != nullptr) {
		if (m_ticksToNextBus == 0) {
			if (hasBuildingFlag("ENABLE_SPACEBUS")) {
				Star* targetStar = HabitatMod::findBusStarDestination(currentStar, faction);;

				if (targetStar->getPlanets().size() > 0) {
					Planet* targetPlanet = HabitatMod::findBusPlanetDestination(m_allegiance, targetStar, planet);

					if (targetPlanet != nullptr) {
						planet->createSpaceBus(faction->getColor(), currentStar, targetStar, targetPlanet);

						m_population -= 1000;
					}
				}
			}
			m_ticksToNextBus = HabitatMod::calcBusTickTimer(m_population);
		}
		else {
			m_ticksToNextBus--;
		}
	}

	// Resource exploitation
	if (faction != nullptr) {
		if (m_ticksToNextResourceExploit == 0) {
			float multiplier = 1.0f;

			for (ColonyBuilding& building : m_buildings) {
				if (building.isBuilt()) {
					multiplier *= building.getExploitationModifer();
				}
			}

			for (Resource& resource : planet->getResources()) {
				float amount = (m_population * resource.abundance / 1000.0f) * multiplier;
				faction->addResource(resource.type, amount);
			}
			m_ticksToNextResourceExploit = 1000;
		}
		else {
			m_ticksToNextResourceExploit--;
		}
	}

	// Update colony buildings
	for (ColonyBuilding& building : m_buildings) {
		building.update();
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

float Colony::getGrowthRate(float planetHabitability) {
	// Apply building modifiers
	for (ColonyBuilding& building : m_buildings) {
		if (building.isBuilt()) {
			planetHabitability *= building.getHabitabilityModifier();
		}
	}
	
	// Negative growth rate if habitability is less than 1.0
	float growthRate = (planetHabitability - 1.0f) / 10.0f;

	return growthRate;
}

bool Colony::hasBuildingFlag(const std::string& flag) {
	for (ColonyBuilding& building : m_buildings) {
		if (building.hasFlag(flag)) {
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

float Colony::getBombardDamageMultipler() const {
	float mult = 1.0f;
	for (const ColonyBuilding& building : m_buildings) {
		mult *= building.getBombardDamageMultipler();
	}
	return mult;
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

float ColonyBuilding::getHabitabilityModifier() const {
	const auto& table = TOMLCache::getTable("data/objects/colonybuildings.toml");
	return table[m_type]["habitabilityModifier"].value_or(1.0f);
}

void ColonyBuilding::update() {
	if (!isBuilt()) {
		m_percentBuilt += 0.01;
	}
}

float ColonyBuilding::getExploitationModifer() const {
	const auto& table = TOMLCache::getTable("data/objects/colonybuildings.toml");
	return table[m_type]["exploitationModifier"].value_or(1.0f);
}

std::string ColonyBuilding::getEffectsString() const {
	std::stringstream effects;
	effects << std::fixed << std::setprecision(1);

	float habitabilityModifier = getHabitabilityModifier();
	float exploitationModifier = getExploitationModifer();
	float bombardDamageMultiplier = getBombardDamageMultipler();

	if (habitabilityModifier != 1.0f) {
		effects << "Habitability: " << Util::percentify(habitabilityModifier, 1) << "\n";
	}

	if (exploitationModifier != 1.0f) {
		effects << "Resource Extraction Rate: " << Util::percentify(exploitationModifier, 1) << "\n";
	}

	if (bombardDamageMultiplier != 1.0f) {
		effects << "Orbital Bombardment Damage Reduction: " << Util::percentify(bombardDamageMultiplier, 1) << "\n";
	}

	for (std::string& flag : getFlags()) {
		if (flag == "ENABLE_SPACEBUS") {
			effects << "Enables Space Bus" << "\n";
		}
	}

	return effects.str();
}

bool ColonyBuilding::hasFlag(const std::string& flag) {
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

float ColonyBuilding::getBombardDamageMultipler() const {
	const toml::table& table = TOMLCache::getTable("data/objects/colonybuildings.toml");

	float mult = table[getType()]["bombardDamageMultiplier"].value_or(1.0f);

	return mult;
}