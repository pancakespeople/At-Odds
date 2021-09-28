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
					multiplier *= building.getEffect("exploitationModifier", 1.0f);
				}
			}

			for (Resource& resource : planet->getResources()) {
				float amount = (m_population * resource.abundance / 1000.0f) * multiplier;
				faction->addResource(resource.type, amount);
			}

			// Trade goods
			float waterHarvest = m_population * 0.01f * planet->getWater();
			m_tradeGoods.addItem("Water", waterHarvest);

			m_ticksToNextResourceExploit = 1000;
		}
		else {
			m_ticksToNextResourceExploit--;
		}
	}

	// Update colony buildings
	for (ColonyBuilding& building : m_buildings) {
		building.update(*this);
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

	if (m_explorationEnabled && faction != nullptr) exploration(planet, faction);
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
			planetHabitability *= building.getEffect("habitabilityModifier", 1.0f);
		}
	}
	
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

void Colony::onBuildingBuild() {
	if (hasBuildingFlag("ENABLE_ORBITAL_CANNON")) {
		m_defenseCannonEnabled = true;
	}

	if (hasBuildingFlag("ENABLE_EXPLORATION")) {
		m_explorationEnabled = true;
		m_explorationEventTimer = Random::randInt(10000, 20000);
	}
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
			Spaceship::DesignerWeapon weapon = faction->addRandomWeapon();
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

void ColonyBuilding::update(Colony& colony) {
	if (!isBuilt()) {
		m_percentBuilt += 0.01;
		if (isBuilt()) colony.onBuildingBuild();
	}
}

std::string ColonyBuilding::getEffectsString() const {
	std::stringstream effects;
	effects << std::fixed << std::setprecision(1);

	float habitabilityModifier = getEffect("habitabilityModifier", 1.0f);
	float exploitationModifier = getEffect("exploitationModifier", 1.0f);
	float bombardDamageMultiplier = getEffect("bombardDamageMultiplier", 1.0f);
	float invasionEffectiveness = getEffect("invasionEffectiveness", 1.0f);

	if (habitabilityModifier != 1.0f) {
		effects << "Habitability: " << Util::percentify(habitabilityModifier, 1) << "\n";
	}

	if (exploitationModifier != 1.0f) {
		effects << "Resource extraction rate: " << Util::percentify(exploitationModifier, 1) << "\n";
	}

	if (bombardDamageMultiplier != 1.0f) {
		effects << "Orbital bombardment damage: " << Util::percentify(bombardDamageMultiplier, 1) << "\n";
	}

	if (invasionEffectiveness != 1.0f) {
		effects << "Invasion effectiveness: " << Util::percentify(invasionEffectiveness, 1) << "\n";
	}

	for (std::string& flag : getFlags()) {
		if (flag == "ENABLE_SPACEBUS") {
			effects << "Enables space bus" << "\n";
		}
		
		if (flag == "ENABLE_ORBITAL_CANNON") {
			effects << "Enables orbital cannon" << "\n";
		}

		if (flag == "ENABLE_EXPLORATION") {
			effects << "Enables exploration events" << "\n";
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
