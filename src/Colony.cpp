#include "gamepch.h"
#include "Colony.h"
#include "Planet.h"
#include "Star.h"
#include "Faction.h"
#include "TOMLCache.h"

bool Colony::isColonizationLegal(int allegiance) {
	if (m_factionColonyLegality.count(allegiance) == 0) return false;
	else return m_factionColonyLegality[allegiance];
}

void Colony::setFactionColonyLegality(int allegiance, bool legality) {
	m_factionColonyLegality[allegiance] = legality;
}

void Colony::update(Star* currentStar, Faction* faction, Planet* planet) {
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
			Star* targetStar = HabitatMod::findBusStarDestination(currentStar, faction);;

			if (targetStar->getPlanets().size() > 0) {
				Planet* targetPlanet = HabitatMod::findBusPlanetDestination(m_allegiance, targetStar, planet);

				if (targetPlanet != nullptr) {
					planet->createSpaceBus(faction->getColor(), currentStar, targetStar, targetPlanet);

					m_population -= 1000;
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
			for (PlanetResource& resource : planet->getResources()) {
				float amount = m_population * resource.abundance / 1000.0f;
				faction->addResource(resource.type, amount);
			}
			m_ticksToNextResourceExploit = 1000;
		}
		else {
			m_ticksToNextResourceExploit--;
		}
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
