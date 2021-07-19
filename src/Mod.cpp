#include "gamepch.h"
#include "Mod.h"
#include "Unit.h"
#include "Star.h"
#include "Faction.h"
#include "Random.h"
#include "Order.h"
#include "Math.h"
#include "Planet.h"
#include "GUI.h"

BOOST_CLASS_EXPORT_GUID(FactoryMod, "FactoryMod")
BOOST_CLASS_EXPORT_GUID(FighterBayMod, "FighterBayMod")
BOOST_CLASS_EXPORT_GUID(HabitatMod, "HabitatMod");

void Mod::openGUI(tgui::ChildWindow::Ptr window, Faction* faction) {
	auto text = tgui::Label::create();
	text->setText(getInfoString());
	window->add(text);
}

void FactoryMod::update(Unit* unit, Star* currentStar, Faction* faction) {
	if (!isEnabled()) return;
	if (faction == nullptr) return;
	
	for (auto& build : m_shipBuildData) {
		if (build.second.build) {
			if (!build.second.resourcesSubtracted) {
				// Check resources
				
				Spaceship::DesignerShip shipDesign = faction->getShipDesignByName(build.first);
				auto cost = shipDesign.getTotalResourceCost();
				
				if (!faction->canSubtractResources(cost)) {
					continue;
				}
				else {
					faction->subtractResources(cost);
					build.second.resourcesSubtracted = true;
				}
			}
			
			if (build.second.progressPercent >= 100.0f) {
				// Spawn the ship
				
				Spaceship::DesignerShip shipDesign = faction->getShipDesignByName(build.first);
				auto shipPtr = std::make_unique<Spaceship>(shipDesign.chassis.type, unit->getPos(), currentStar, faction->getID(), faction->getColor());
				
				// Add weapons
				for (Spaceship::DesignerWeapon& weapon : shipDesign.weapons) {
					shipPtr->addWeapon(weapon.type);
				}

				faction->addSpaceship(currentStar->createSpaceship(shipPtr));

				DEBUG_PRINT("Created spaceship " + shipDesign.chassis.type);
				
				if (m_buildProgressBar != nullptr && build.second.selected) m_buildProgressBar->setValue(0);
				
				build.second.progressPercent = 0.0f;
				build.second.resourcesSubtracted = false;
			}
			else {
				build.second.progressPercent += 0.05;

				if (m_buildProgressBar != nullptr && build.second.selected) {
					m_buildProgressBar->setValue(build.second.progressPercent);
				}
			}
		}
	}
}

std::string FactoryMod::getInfoString() {
	return "";
}

void FactoryMod::openGUI(tgui::ChildWindow::Ptr window, Faction* faction) {
	window->setSize("25%", "25%");

	auto designsListBox = tgui::ListBox::create();
	designsListBox->setPosition("2.5%", "10%");
	designsListBox->setSize("33% - 2.5%", "90% - 2.5%");
	designsListBox->onItemSelect([this ,designsListBox, window, faction]() {
		auto shipWidgets = window->get<tgui::Group>("shipWidgets");
		
		if (designsListBox->getSelectedItemIndex() != -1) {
			Spaceship::DesignerShip ship = faction->getShipDesignByName(designsListBox->getSelectedItem().toStdString());

			if (ship.name == "") return;

			auto totalResourceCost = ship.getTotalResourceCost();
			shipWidgets->removeAllWidgets();

			auto shipInfoGroup = tgui::Group::create();
			shipInfoGroup->setPosition("parent.designsListBox.right + 2.5%", "10%");
			shipInfoGroup->setSize("33% - 2.5%", "90% - 2.5%");
			shipWidgets->add(shipInfoGroup, "shipInfoGroup");

			ShipDesignerGUI::displayShipResourceCost(shipInfoGroup, totalResourceCost, 0);

			auto buildCheckbox = tgui::CheckBox::create("Build");
			buildCheckbox->setPosition("shipInfoGroup.right + 2.5%", "shipInfoGroup.top");
			buildCheckbox->onChange([this, ship, buildCheckbox]() {
				if (m_shipBuildData.count(ship.name) > 0) {
					m_shipBuildData[ship.name].build = buildCheckbox->isChecked();
				}
			});
			shipWidgets->add(buildCheckbox);

			if (m_shipBuildData.count(ship.name) == 0) {
				ShipBuildData data;
				m_shipBuildData[ship.name] = data;
			}
			else {
				buildCheckbox->setChecked(m_shipBuildData[ship.name].build);
			}

			for (auto& data : m_shipBuildData) {
				if (data.first == ship.name) {
					data.second.selected = true;
				}
				else {
					data.second.selected = false;
				}
			}

			m_buildProgressBar = tgui::ProgressBar::create();
			m_buildProgressBar->setPosition("parent.designsListBox.right + 2.5%", "80%");
			m_buildProgressBar->setSize("66% - 5%", "10%");
			m_buildProgressBar->setValue(m_shipBuildData[ship.name].progressPercent);
			shipWidgets->add(m_buildProgressBar);
		}
		else {
			shipWidgets->removeAllWidgets();
			m_buildProgressBar = nullptr;
		}
	});
	window->add(designsListBox, "designsListBox");
	
	auto designsLabel = tgui::Label::create("Designs");
	designsLabel->setOrigin(0.5f, 0.0f);
	designsLabel->setPosition("designsListBox.width / 2 + designsListBox.left", "0%");
	window->add(designsLabel);

	for (Spaceship::DesignerShip& ship : faction->getShipDesigns()) {
		designsListBox->addItem(ship.name);
	}
	
	auto shipWidgets = tgui::Group::create();
	window->add(shipWidgets, "shipWidgets");
}

void FactoryMod::updateDesigns(Faction* faction) {
	for (Spaceship::DesignerShip ship : faction->getShipDesigns()) {
		if (m_shipBuildData.count(ship.name) == 0) {
			ShipBuildData data;
			m_shipBuildData[ship.name] = data;
		}
	}
}

void FactoryMod::setBuildAll(bool build) {
	for (auto& data : m_shipBuildData) {
		data.second.build = build;
	}
}

FighterBayMod::FighterBayMod(const Unit* unit, Star* star, int allegiance, sf::Color color) {
	for (int i = 0; i < 4; i++) {
		float radius = unit->getCollider().getRadius();
		auto ship = std::make_unique<Spaceship>(
			"FIGHTER", unit->getPos() + Random::randVec(-radius, radius), star, allegiance, color);
		
		if (Random::randBool()) {
			ship->addWeapon(Weapon("LASER_GUN"));
		}
		else {
			ship->addWeapon(Weapon("MACHINE_GUN"));
		}

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
			"FIGHTER", unit->getPos() + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getFactionColor());

		if (Random::randBool()) {
			ship->addWeapon(Weapon("LASER_GUN"));
		}
		else {
			ship->addWeapon(Weapon("MACHINE_GUN"));
		}

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
	// Transfer passengers to planet
	
	bool firstTime = planet->getColony().population == 0;
	planet->getColony().population += m_population;
	if (firstTime && m_population > 0) {
		planet->getColony().allegiance = unit->getAllegiance();
		planet->getColony().factionColor = unit->getFactionColor();
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
		std::make_unique<Spaceship>("SPACE_BUS", pos, currentStar, allegiance, color)
	);
	bus->addMod(HabitatMod(1000, 1000, false));
	bus->addOrder(TravelOrder(targetStar));
	bus->addOrder(InteractWithPlanetOrder(targetPlanet, targetStar));
	bus->addOrder(TravelOrder(currentStar));
	bus->addOrder(FlyToOrder(pos));
	bus->addOrder(DieOrder(true));
}

void HabitatMod::createSpaceBus(Unit* unit, Star* currentStar, Star* targetStar, Planet* targetPlanet) {
	createSpaceBus(unit->getPos(), unit->getAllegiance(), unit->getFactionColor(), currentStar, targetStar, targetPlanet);
}
