#include "gamepch.h"
#include "Mod.h"
#include "Unit.h"
#include "Star.h"
#include "Faction.h"
#include "Random.h"
#include "Order.h"
#include "Math.h"
#include "Planet.h"

BOOST_CLASS_EXPORT_GUID(FactoryMod, "FactoryMod")
BOOST_CLASS_EXPORT_GUID(FighterBayMod, "FighterBayMod")
BOOST_CLASS_EXPORT_GUID(HabitatMod, "HabitatMod");

void FactoryMod::update(Unit* unit, Star* currentStar, Faction* faction) {
	if (!isEnabled()) return;
	
	if (m_ticksToNextShip <= 0) {
		sf::Vector2f pos = unit->getPos();
		float radius = unit->getCollider().getRadius();
		
		if (m_numShips % 10 == 0 && m_numShips != 0) {
			Spaceship* destroyer = currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::DESTROYER_1, pos + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor()));
			if (faction != nullptr) faction->addSpaceship(destroyer);

			Spaceship* constrShip = currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::CONSTRUCTION_SHIP, pos + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor()));
			if (faction != nullptr) faction->addSpaceship(constrShip);
		}
		else {
			Spaceship* frigate = currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::FRIGATE_1, pos + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor()));
			if (faction != nullptr) faction->addSpaceship(frigate);
		}

		m_numShips++;
		m_ticksToNextShip = 2000;
	}
	else {
		m_ticksToNextShip -= 1;
	}
}

std::string FactoryMod::getInfoString() {
	return "Next ship: " + std::to_string(m_ticksToNextShip / 60.0f) + "s";
}

FighterBayMod::FighterBayMod(const Unit* unit, Star* star, int allegiance, sf::Color color) {
	for (int i = 0; i < 4; i++) {
		float radius = unit->getCollider().getRadius();
		auto ship = std::make_unique<Spaceship>(
			Spaceship::SPACESHIP_TYPE::FIGHTER, unit->getPos() + Random::randVec(-radius, radius), star, allegiance, color);
		
		Spaceship* shipPtr = star->createSpaceship(ship);
		shipPtr->disable();
		
		m_fighterShipIds.push_back(shipPtr->getID());
	}
}

void FighterBayMod::update(Unit* unit, Star* currentStar, Faction* faction) {
	if (unit->isDead()) {
		if (!isEnabled()) {
			killAllFighters(currentStar); // They just die if the building was never constructed, no free fighters!
		}
		else {
			launchFighters(currentStar); // Pop out fighters
		}
		return;
	}

	if (!isEnabled()) return;
	
	int numEnemyUnits = unit->findEnemyUnits().size();
	
	if (numEnemyUnits > 0 && m_fighterStatus == FIGHTER_STATUS::DOCKED) {
		// Enable fighters if enemies are in system
		
		launchFighters(currentStar);
	}
	
	if (numEnemyUnits == 0 && m_fighterStatus == FIGHTER_STATUS::FIGHTING) {
		//Recall fighters

		recallFighters(currentStar, unit);
	}
	
	if (m_fighterStatus == FIGHTER_STATUS::RETURNING) {
		// Disable fighters that have returned until all are docked
		
		dockReturningFighters(currentStar, unit);
	}

	// Delete ids for dead ships
	for (int i = 0; i < m_fighterShipIds.size(); i++) {
		if (currentStar->getShipByID(m_fighterShipIds[i]) == nullptr) {
			m_fighterShipIds.erase(m_fighterShipIds.begin() + i);
			i--;
		}
	}

	if (m_fighterShipIds.size() < m_maxFighters) {
		constructNewFighter(currentStar, unit);
	}
}

void FighterBayMod::launchFighters(Star* currentStar) {
	for (int i = 0; i < m_fighterShipIds.size(); i++) {
		Spaceship* fighter = currentStar->getShipByID(m_fighterShipIds[i]);
		if (fighter != nullptr) {
			fighter->enable();
		}
	}

	m_fighterStatus = FIGHTER_STATUS::FIGHTING;
}

void FighterBayMod::recallFighters(Star* currentStar, Unit* unit) {
	for (int i = 0; i < m_fighterShipIds.size(); i++) {
		Spaceship* fighter = currentStar->getShipByID(m_fighterShipIds[i]);
		if (fighter != nullptr) {
			fighter->clearOrders();
			fighter->addOrder(FlyToOrder(unit->getPos()));
		}
	}

	m_fighterStatus = FIGHTER_STATUS::RETURNING;
}

void FighterBayMod::dockReturningFighters(Star* currentStar, Unit* unit) {
	int numReturningFighters = 0;

	for (int i = 0; i < m_fighterShipIds.size(); i++) {
		Spaceship* fighter = currentStar->getShipByID(m_fighterShipIds[i]);
		if (fighter != nullptr) {
			if (!fighter->isDisabled()) {
				if (Math::distance(fighter->getPos(), unit->getPos()) < unit->getCollider().getRadius()) {
					fighter->disable();
				}
				else {
					numReturningFighters++;
				}
			}
		}
	}

	if (m_fighterShipIds.size() == 0 || numReturningFighters == 0) {
		m_fighterStatus = FIGHTER_STATUS::DOCKED;
	}
}

void FighterBayMod::constructNewFighter(Star* currentStar, Unit* unit) {
	if (m_ticksToNextFighter == 0) {
		float radius = unit->getCollider().getRadius();
		auto ship = std::make_unique<Spaceship>(
			Spaceship::SPACESHIP_TYPE::FIGHTER, unit->getPos() + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getFactionColor());

		Spaceship* shipPtr = currentStar->createSpaceship(ship);
		
		if (m_fighterStatus != FIGHTER_STATUS::FIGHTING) {
			shipPtr->disable();
		}

		m_fighterShipIds.push_back(shipPtr->getID());

		m_ticksToNextFighter = 1000;

		DEBUG_PRINT("constructed new fighter");
	}
	else {
		m_ticksToNextFighter--;
	}
}

void FighterBayMod::killAllFighters(Star* currentStar) {
	for (int i = 0; i < m_fighterShipIds.size(); i++) {
		Spaceship* fighter = currentStar->getShipByID(m_fighterShipIds[i]);
		if (fighter != nullptr) {
			fighter->enable();
			fighter->kill();
		}
	}
	m_fighterShipIds.clear();
}

std::string FighterBayMod::getInfoString() {
	return "Fighters: " + std::to_string(m_fighterShipIds.size());
}

HabitatMod::HabitatMod(int population, int maxPopulation, bool spawnsSpaceBus) {
	m_population = population;
	m_popCap = maxPopulation;
	m_spawnsSpaceBus = spawnsSpaceBus;
	m_ticksToNextBus = calcBusTickTimer();
}

void HabitatMod::update(Unit* unit, Star* currentStar, Faction* faction) {
	if (m_ticksToNextGrowth == 0) {
		if (m_population != m_popCap) {
			m_population += m_population * m_growthRate;
		}

		if (m_population > m_popCap) {
			m_population = m_popCap;
		}
		
		m_ticksToNextGrowth = 1000;
	}
	else {
		m_ticksToNextGrowth--;
	}

	if (m_spawnsSpaceBus) {
		if (m_ticksToNextBus == 0) {
			Star* targetStar = HabitatMod::findBusStarDestination(currentStar, faction);;

			if (targetStar->getPlanets().size() > 0) {
				Planet* targetPlanet = HabitatMod::findBusPlanetDestination(targetStar);
				
				if (targetPlanet != nullptr) {
					HabitatMod::createSpaceBus(unit, currentStar, targetStar, targetPlanet);

					m_population -= 1000;
				}
			}
			m_ticksToNextBus = calcBusTickTimer();
		}
		else {
			m_ticksToNextBus--;
		}
	}
}

std::string HabitatMod::getInfoString() {
	std::string info;
	info += "Population: " + std::to_string(m_population);
	return info;
}

void HabitatMod::interactWithPlanet(Unit* unit, Planet* planet) {
	planet->getColony().population += m_population;
	m_population = 0;
	if (planet->getColony().allegiance == -1) {
		planet->getColony().allegiance = unit->getAllegiance();
	}
}

int HabitatMod::calcBusTickTimer() {
	if (m_population == 0) return 3000;
	return 50000000 / m_population;
}

Star* HabitatMod::findBusStarDestination(Star* currentStar, Faction* faction) {
	Star* targetStar = nullptr;
	if (Random::randBool()) {
		// Half the time, use the home system
		targetStar = currentStar;
	}
	else {
		// Head to a random star
		auto& stars = faction->getOwnedStars();

		if (stars.size() > 0) {
			int rndIndex = Random::randInt(0, stars.size() - 1);
			targetStar = stars[rndIndex];
		}
	}

	if (targetStar == nullptr) {
		targetStar = currentStar;
	}
	return targetStar;
}

Planet* HabitatMod::findBusPlanetDestination(Star* targetStar) {
	Planet* targetPlanet = nullptr;
	std::vector<Planet>& planets = targetStar->getPlanets();

	if (Random::randBool()) {
		// Half of the time, just head to the most habitable planet
		targetPlanet = &targetStar->getMostHabitablePlanet();
		DEBUG_PRINT("Colonizing most habitable planet");
	}
	else {
		Planet* mostHabitable = &targetStar->getMostHabitablePlanet();

		// 10 attempts to find a planet
		for (int i = 0; i < 10; i++) {
			Planet* randPlanet = &planets[Random::randInt(0, planets.size() - 1)];
			if (randPlanet != mostHabitable) {
				if (randPlanet->getType() != Planet::PLANET_TYPE::GAS_GIANT &&
					randPlanet->getType() != Planet::PLANET_TYPE::ICE_GIANT &&
					Random::randFloat(0.0f, 1.0f) < 0.9f) {
					// Colonize a terrestrial planet 90% of the time
					targetPlanet = randPlanet;
					DEBUG_PRINT("Colonizing random terrestial planet");
					break;
				}
				else {
					// Colonize a gas giant
					targetPlanet = randPlanet;
					DEBUG_PRINT("Colonizing gas giant");
					break;
				}

			}
		}
	}
	return targetPlanet;
}

void HabitatMod::createSpaceBus(Unit* unit, Star* currentStar, Star* targetStar, Planet* targetPlanet) {
	Spaceship* bus = currentStar->createSpaceship(
		std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::SPACE_BUS, unit->getPos(), currentStar, unit->getAllegiance(), unit->getFactionColor())
	);
	bus->addOrder(TravelOrder(targetStar));
	bus->addOrder(InteractWithPlanetOrder(targetPlanet, targetStar));
	bus->addOrder(TravelOrder(currentStar));
	bus->addOrder(FlyToOrder(unit->getPos()));
	bus->addOrder(DieOrder(true));
}