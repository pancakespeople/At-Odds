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

void Mod::openGUI(tgui::ChildWindow::Ptr window) {
	auto text = tgui::Label::create();
	text->setText(getInfoString());
	window->add(text);
}

void FactoryMod::update(Unit* unit, Star* currentStar, Faction* faction) {
	if (!isEnabled()) return;
	if (faction == nullptr) return;
	
	if (m_ticksToNextShip <= 0) {
		sf::Vector2f pos = unit->getPos();
		float radius = unit->getCollider().getRadius();
		if (faction->canSubtractResource(PlanetResource::RESOURCE_TYPE::COMMON_ORE, 25) &&
			faction->canSubtractResource(PlanetResource::RESOURCE_TYPE::UNCOMMON_ORE, 5) &&
			m_buildDestroyer) {
			Spaceship* destroyer = currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::DESTROYER_1, pos + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor()));
			faction->addSpaceship(destroyer);

			faction->subtractResource(PlanetResource::RESOURCE_TYPE::COMMON_ORE, 25);
			faction->subtractResource(PlanetResource::RESOURCE_TYPE::UNCOMMON_ORE, 5);
		}

		if (faction->canSubtractResource(PlanetResource::RESOURCE_TYPE::COMMON_ORE, 33) &&
			m_buildConstructor) {
			Spaceship* constrShip = currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::CONSTRUCTION_SHIP, pos + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor()));
			faction->addSpaceship(constrShip);

			faction->subtractResource(PlanetResource::RESOURCE_TYPE::COMMON_ORE, 33);
		}
		if (faction->canSubtractResource(PlanetResource::RESOURCE_TYPE::COMMON_ORE, 10) &&
			m_buildFrigate) {
			Spaceship* frigate = currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::FRIGATE_1, pos + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor()));
			faction->addSpaceship(frigate);

			faction->subtractResource(PlanetResource::RESOURCE_TYPE::COMMON_ORE, 10);
		}

		m_ticksToNextShip = 500;
	}
	else {
		m_ticksToNextShip -= 1;
	}
}

std::string FactoryMod::getInfoString() {
	return "Next ship: " + std::to_string(m_ticksToNextShip / 60.0f) + "s";
}

void FactoryMod::openGUI(tgui::ChildWindow::Ptr window) {
	window->setSize("25%", "25%");

	auto frigLabel = tgui::Label::create("Frigate: ");
	window->add(frigLabel, "frigLabel");

	auto frigCost = tgui::Label::create("Cost: 10 Kathium");
	frigCost->setPosition("frigLabel.left", "frigLabel.bottom");
	window->add(frigCost);

	auto frigCheckbox = tgui::CheckBox::create();
	frigCheckbox->setPosition("frigLabel.right", "frigLabel.top");
	frigCheckbox->setChecked(m_buildFrigate);
	frigCheckbox->onChange([this, frigCheckbox]() {
		m_buildFrigate = frigCheckbox->isChecked();
	});
	window->add(frigCheckbox);

	auto destrLabel = tgui::Label::create("Destroyer: ");
	destrLabel->setPosition("0%", "33%");
	window->add(destrLabel, "destrLabel");

	auto destrCost = tgui::Label::create("Cost: 25 Kathium, 5 Oscillite");
	destrCost->setPosition("destrLabel.left", "destrLabel.bottom");
	window->add(destrCost);

	auto destrCheckbox = tgui::CheckBox::create();
	destrCheckbox->setPosition("destrLabel.right", "destrLabel.top");
	destrCheckbox->setChecked(m_buildDestroyer);
	destrCheckbox->onChange([this, destrCheckbox]() {
		m_buildDestroyer = destrCheckbox->isChecked();
	});
	window->add(destrCheckbox);

	auto csLabel = tgui::Label::create("Constructor: ");
	csLabel->setPosition("0%", "66%");
	window->add(csLabel, "csLabel");

	auto csCost = tgui::Label::create("Cost: 33 Kathium");
	csCost->setPosition("csLabel.left", "csLabel.bottom");
	window->add(csCost);

	auto csCheckbox = tgui::CheckBox::create();
	csCheckbox->setPosition("csLabel.right", "csLabel.top");
	csCheckbox->setChecked(m_buildConstructor);
	csCheckbox->onChange([this, csCheckbox]() {
		m_buildConstructor = csCheckbox->isChecked();
		});
	window->add(csCheckbox);
}

void FactoryMod::setBuild(bool frigate, bool destroyer, bool constructor) {
	m_buildFrigate = frigate;
	m_buildDestroyer = destroyer;
	m_buildConstructor = constructor;
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

void FighterBayMod::onUnitDeath(Star* currentStar) {
	if (!isEnabled()) {
		killAllFighters(currentStar); // They just die if the building was never constructed, no free fighters!
	}
	else {
		launchFighters(currentStar); // Pop out fighters
	}
}

HabitatMod::HabitatMod(int population, int maxPopulation, bool spawnsSpaceBus) {
	m_population = population;
	m_popCap = maxPopulation;
	m_spawnsSpaceBus = spawnsSpaceBus;
	m_ticksToNextBus = HabitatMod::calcBusTickTimer(m_population);
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
				Planet* targetPlanet = HabitatMod::findBusPlanetDestination(unit->getAllegiance(), targetStar);
				
				if (targetPlanet != nullptr) {
					HabitatMod::createSpaceBus(unit, currentStar, targetStar, targetPlanet);

					m_population -= 1000;
				}
			}
			m_ticksToNextBus = HabitatMod::calcBusTickTimer(m_population);
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
	bool firstTime = planet->getColony().population == 0;
	planet->getColony().population += m_population;
	if (firstTime && m_population > 0) {
		planet->getColony().allegiance = unit->getAllegiance();
		planet->onColonization();
	}
	m_population = 0;
}

int HabitatMod::calcBusTickTimer(int population) {
	if (population == 0) return 3000;
	int timer = 50000000 / population;
	
	if (timer > 3000) return 3000;
	if (timer < 120) return 120;
	else return timer;
}

Star* HabitatMod::findBusStarDestination(Star* currentStar, Faction* faction) {
	Star* targetStar = nullptr;
	if (Random::randBool() && currentStar->getPlanets().size() != 0) {
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

Planet* HabitatMod::findBusPlanetDestination(int allegiance, Star* targetStar, Planet* avoidPlanet) {
	Planet* targetPlanet = nullptr;
	std::vector<Planet>& planets = targetStar->getPlanets();

	Planet* mostHabitable = &targetStar->getMostHabitablePlanet();

	if (Random::randBool() && mostHabitable != avoidPlanet) {
		// Half of the time, just head to the most habitable planet
		if (mostHabitable->getColony().isColonizationLegal(allegiance)) {
			targetPlanet = mostHabitable;
		}
	}
	else {
		// 10 attempts to find a planet
		for (int i = 0; i < 10; i++) {
			Planet* randPlanet = &planets[Random::randInt(0, planets.size() - 1)];
			if (randPlanet != avoidPlanet && randPlanet->getColony().isColonizationLegal(allegiance)) {
				if (randPlanet != mostHabitable) {
					if (randPlanet->getType() != Planet::PLANET_TYPE::GAS_GIANT &&
						randPlanet->getType() != Planet::PLANET_TYPE::ICE_GIANT &&
						Random::randFloat(0.0f, 1.0f) < 0.9f) {
						// Colonize a terrestrial planet 90% of the time
						targetPlanet = randPlanet;
						break;
					}
					else {
						// Colonize a gas giant
						targetPlanet = randPlanet;
						break;
					}

				}
			}
		}
	}
	return targetPlanet;
}

void HabitatMod::createSpaceBus(sf::Vector2f pos, int allegiance, sf::Color color, Star* currentStar, Star* targetStar, Planet* targetPlanet) {
	Spaceship* bus = currentStar->createSpaceship(
		std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::SPACE_BUS, pos, currentStar, allegiance, color)
	);
	bus->addOrder(TravelOrder(targetStar));
	bus->addOrder(InteractWithPlanetOrder(targetPlanet, targetStar));
	bus->addOrder(TravelOrder(currentStar));
	bus->addOrder(FlyToOrder(pos));
	bus->addOrder(DieOrder(true));
}

void HabitatMod::createSpaceBus(Unit* unit, Star* currentStar, Star* targetStar, Planet* targetPlanet) {
	createSpaceBus(unit->getPos(), unit->getAllegiance(), unit->getFactionColor(), currentStar, targetStar, targetPlanet);
}
