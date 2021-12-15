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
#include "GUI/ShipDesigner.h"
#include "Sounds.h"
#include "TradeGoods.h"
#include "Util.h"
#include "Constellation.h"
#include "Pathfinder.h"
#include "Spaceship.h"

BOOST_CLASS_EXPORT_GUID(FactoryMod, "FactoryMod")
BOOST_CLASS_EXPORT_GUID(FighterBayMod, "FighterBayMod")
BOOST_CLASS_EXPORT_GUID(HabitatMod, "HabitatMod");
BOOST_CLASS_EXPORT_GUID(TradeMod, "TradeMod");
BOOST_CLASS_EXPORT_GUID(ScienceMod, "ScienceMod");
BOOST_CLASS_EXPORT_GUID(PirateBaseMod, "PirateBaseMod");

void Mod::openGUI(tgui::ChildWindow::Ptr window, Faction* faction) {
	auto text = tgui::Label::create();
	text->setText(getInfoString());
	window->add(text);
}

void FactoryMod::update(Unit* unit, Star* currentStar, Faction* faction) {
	if (!isEnabled()) return;
	if (faction == nullptr) return;
	
	// Find the first build in queue
	int buildIndex = -1;
	for (int i = 0; i < m_shipBuildData.size(); i++) {
		if (m_shipBuildData[i].build) {
			buildIndex = i;
			break;
		}
	}

	if (buildIndex != -1) {
		ShipBuildData& build = m_shipBuildData[buildIndex];
		// Stop production if amount equals 0
		if (build.amount == 0 && !build.continuous) {
			build.build = false;

			// Update gui
			if (m_shipWidgets != nullptr && build.selected) {
				auto buildCheckbox = m_shipWidgets->get<tgui::CheckBox>("buildCheckbox");
				if (buildCheckbox != nullptr) buildCheckbox->setChecked(false);
			}
		}
		
		if (!build.resourcesSubtracted && build.build) {
			// Check resources
				
			DesignerShip shipDesign = faction->getShipDesignByName(build.shipName);
			auto cost = shipDesign.getTotalResourceCost();
				
			if (faction->canSubtractResources(cost)) {
				faction->subtractResources(cost);
				build.resourcesSubtracted = true;
				build.buildTimeMultiplier = shipDesign.chassis.buildTimeMultiplier;
			}
		}
			
		if (build.progressPercent >= 100.0f) {
			// Spawn the ship
				
			DesignerShip shipDesign = faction->getShipDesignByName(build.shipName);
			Spaceship* shipPtr = currentStar->createSpaceship(shipDesign.chassis.type, unit->getPos(), faction->getID(), faction->getColor());
				
			// Add weapons
			for (DesignerWeapon& weapon : shipDesign.weapons) {
				shipPtr->addWeapon(weapon.type);
			}

			// Add construction gun
			if (shipPtr->getConstructionSpeed() > 0.0f) {
				shipPtr->addWeapon(Weapon("CONSTRUCTION_GUN"));
			}

			sf::Vector2f randVel = Random::randVec(-50.0f, 50.0f);
			shipPtr->addVelocity(randVel);

			faction->addSpaceship(shipPtr);

			DEBUG_PRINT("Created spaceship " + shipDesign.chassis.type);
				
			if (m_buildProgressBar != nullptr && build.selected) m_buildProgressBar->setValue(0);
				
			build.progressPercent = 0.0f;
			build.resourcesSubtracted = false;

			if (!build.continuous) {
				build.amount--;
					
				// Update gui
				if (m_shipWidgets != nullptr && build.selected) {
					auto amountEditBox = m_shipWidgets->get<tgui::EditBox>("amountEditBox");
					if (amountEditBox != nullptr) amountEditBox->setText(std::to_string(build.amount));
				}
			}

			// Move build to the back of the queue
			m_shipBuildData.push_back(build);
			m_shipBuildData.erase(m_shipBuildData.begin() + buildIndex);
			
			// Update design list box
			if (m_designsListBox != nullptr) {
				int newIndex = m_designsListBox->getSelectedItemIndex() - 1;
				if (newIndex == -1) {
					newIndex = m_designsListBox->getItemCount() - 1;
				}
				updateDesignsListBox(newIndex);
			}
		}
		else if (build.resourcesSubtracted) {
			build.progressPercent += 0.05f / build.buildTimeMultiplier * getBuildSpeedMultiplier();
			if (m_weaponsStockpile > 0.0f) m_weaponsStockpile -= 0.01f * getBuildSpeedMultiplier();
			if (m_weaponsStockpile < 0.0f) m_weaponsStockpile = 0.0f;

			if (m_buildProgressBar != nullptr && build.selected) {
				m_buildProgressBar->setValue(build.progressPercent);
			}

			if (m_armamentsLabel != nullptr) m_armamentsLabel->setText("Armaments: " + Util::cutOffDecimal(m_weaponsStockpile, 2) + "/100");
			if (m_buildSpeedLabel != nullptr) m_buildSpeedLabel->setText("Additional build speed: " + Util::percentify(getBuildSpeedMultiplier(), 2));
		}
	}

	// Stockpile weapons
	if (m_checkForWeaponsTimer == 0) {
		if (m_weaponsStockpile < 100.0f) {
			Planet* mostSupplyPlanet = nullptr;
			float mostSupply = 0.0f;
			
			for (Planet& planet : currentStar->getPlanets()) {
				auto& goods = planet.getColony().getTradeGoods().getGoods();

				if (planet.getColony().getAllegiance() == faction->getID()) {
					if (goods.count("ARMAMENTS") > 0) {
						

						if (goods.at("ARMAMENTS").supply > mostSupply) {
							mostSupplyPlanet = &planet;
							mostSupply = goods.at("ARMAMENTS").supply;
						}
					}
				}
			}

			if (mostSupply > 0.0f) {
				// Send space truck

				Spaceship* ship = currentStar->createSpaceship("SPACE_TRUCK", mostSupplyPlanet->getPos(), faction->getID(), faction->getColor());
				ship->addOrder(InteractWithUnitOrder(unit));
				ship->addOrder(DieOrder(true));

				TradeMod mod;
				mod.addItem("ARMAMENTS", std::min(100.0f, mostSupply), 0.0f);
				mostSupplyPlanet->getColony().getTradeGoods().removeSupply("ARMAMENTS", std::min(100.0f, mostSupply), true);

				ship->addMod(mod);

				DEBUG_PRINT("Sent space truck to arm ship factory in " << currentStar->getName());
			}
		}
		
		m_checkForWeaponsTimer = 1000;
	}
	else m_checkForWeaponsTimer--;
}

void FactoryMod::onShipInteract(Spaceship* ship) {
	TradeMod* tradeMod = ship->getMod<TradeMod>();
	if (tradeMod != nullptr) {
		if (tradeMod->getGoods().count("ARMAMENTS") > 0) {
			m_weaponsStockpile += tradeMod->getGoods().at("ARMAMENTS");
			tradeMod->setItem("ARMAMENTS", 0.0f);
			DEBUG_PRINT("New weapon stockpile: " << m_weaponsStockpile);
		}
	}
}

std::string FactoryMod::getInfoString() {
	return "";
}

void FactoryMod::openGUI(tgui::ChildWindow::Ptr window, Faction* faction) {
	window->setSize("50%", "50%");
	window->onClose([this]() {
		m_shipWidgets = nullptr;
		for (auto& data : m_shipBuildData) {
			data.selected = false;
		}
	});

	m_designsListBox = tgui::ListBox::create();
	m_designsListBox->setPosition("2.5%", "5%");
	m_designsListBox->setSize("33% - 2.5%", "50% - 2.5%");
	m_designsListBox->onItemSelect([this, window, faction]() {
		auto shipWidgets = window->get<tgui::Group>("shipWidgets");
		
		if (m_designsListBox->getSelectedItemIndex() != -1) {
			DesignerShip ship = faction->getShipDesignByName(m_designsListBox->getSelectedItem().toStdString());
			int buildIndex = -1;

			// Find build index
			for (int i = 0; i < m_shipBuildData.size(); i++) {
				if (m_shipBuildData[i].shipName == ship.name) buildIndex = i;
			}

			if (ship.name == "") return;

			auto totalResourceCost = ship.getTotalResourceCost();
			shipWidgets->removeAllWidgets();

			auto shipInfoGroup = tgui::Group::create();
			shipInfoGroup->setPosition("parent.designsListBox.right + 2.5%", "10%");
			shipInfoGroup->setSize("33% - 2.5%", "90% - 2.5%");
			shipWidgets->add(shipInfoGroup, "shipInfoGroup");

			GUIUtil::displayResourceCost(shipInfoGroup, totalResourceCost, 0, 10);

			auto buildCheckbox = tgui::CheckBox::create("Build");
			buildCheckbox->setPosition("shipInfoGroup.right + 2.5%", "shipInfoGroup.top");
			shipWidgets->add(buildCheckbox, "buildCheckbox");
			
			auto continuousCheckbox = tgui::CheckBox::create("Continuous");
			continuousCheckbox->setPosition("shipInfoGroup.right + 2.5%", "shipInfoGroup.top + 15%");
			shipWidgets->add(continuousCheckbox);

			auto amountEditBox = tgui::EditBox::create();
			amountEditBox->setPosition("shipInfoGroup.right + 2.5%", "shipInfoGroup.top + 30%");
			amountEditBox->setInputValidator("[0-9]+");
			amountEditBox->setSize("10%", "10%");
			amountEditBox->setMaximumCharacters(2);
			amountEditBox->setText("1");
			shipWidgets->add(amountEditBox, "amountEditBox");

			auto amountLabel = tgui::Label::create("Amount");
			amountLabel->setPosition("amountEditBox.right + 1%", "amountEditBox.top");
			shipWidgets->add(amountLabel);

			auto moveDownButton = tgui::Button::create("Queue Down");
			moveDownButton->setPosition("33% + 2.5%", "50%");
			moveDownButton->setSize("15%", "5%");
			shipWidgets->add(moveDownButton, "moveUpButton");

			auto moveUpButton = tgui::Button::create("Queue Up");
			moveUpButton->setPosition("33% + 2.5%", "moveUpButton.top - moveUpButton.height");
			moveUpButton->setSize("15%", "5%");
			shipWidgets->add(moveUpButton);

			// Create ship build data if it doesnt exist or init the widget values with the data if it does
			if (buildIndex == -1) {
				ShipBuildData data(ship.name);
				m_shipBuildData.push_back(data);

				buildIndex = m_shipBuildData.size() - 1;
			}
			else {
				buildCheckbox->setChecked(m_shipBuildData[buildIndex].build);
				continuousCheckbox->setChecked(m_shipBuildData[buildIndex].continuous);
				amountEditBox->setText(std::to_string(m_shipBuildData[buildIndex].amount));
			}

			// Set selected design
			for (auto& data : m_shipBuildData) {
				if (data.shipName == ship.name) {
					data.selected = true;
				}
				else {
					data.selected = false;
				}
			}

			m_buildProgressBar = tgui::ProgressBar::create();
			m_buildProgressBar->setPosition("parent.designsListBox.right + 2.5%", "80%");
			m_buildProgressBar->setSize("66% - 5%", "10%");
			m_buildProgressBar->setValue(m_shipBuildData[buildIndex].progressPercent);
			shipWidgets->add(m_buildProgressBar);

			// Callbacks
			amountEditBox->onUnfocus([this, ship, buildIndex, amountEditBox]() {
				if (amountEditBox->getText().size() > 0) {
					m_shipBuildData[buildIndex].amount = amountEditBox->getText().toInt();
				}
			});

			continuousCheckbox->onChange([this, ship, buildIndex, continuousCheckbox]() {
				m_shipBuildData[buildIndex].continuous = continuousCheckbox->isChecked();
			});

			buildCheckbox->onChange([this, ship, buildIndex, buildCheckbox]() {
				m_shipBuildData[buildIndex].build = buildCheckbox->isChecked();
				updateDesignsListBox(buildIndex);
			});

			moveUpButton->onClick([this, buildIndex]() {
				if (buildIndex != 0) {
					// Swap elements
					ShipBuildData before = m_shipBuildData[static_cast<size_t>(buildIndex) - 1];
					m_shipBuildData[static_cast<size_t>(buildIndex) - 1] = m_shipBuildData[buildIndex];
					m_shipBuildData[buildIndex] = before;
					updateDesignsListBox(buildIndex - 1);
				}
			});

			moveDownButton->onClick([this, buildIndex]() {
				if (buildIndex != m_shipBuildData.size() - 1) {
					// Swap elements
					ShipBuildData after = m_shipBuildData[static_cast<size_t>(buildIndex) + 1];
					m_shipBuildData[static_cast<size_t>(buildIndex) + 1] = m_shipBuildData[buildIndex];
					m_shipBuildData[buildIndex] = after;
					updateDesignsListBox(buildIndex + 1);
				}
			});
		}
		else {
			shipWidgets->removeAllWidgets();
			m_buildProgressBar = nullptr;
			m_shipWidgets = nullptr;
		}
	});
	window->add(m_designsListBox, "designsListBox");
	
	auto designsLabel = tgui::Label::create("Design Queue");
	designsLabel->setOrigin(0.5f, 0.0f);
	designsLabel->setPosition("designsListBox.width / 2 + designsListBox.left", "0%");
	window->add(designsLabel);

	updateDesigns(faction);
	updateDesignsListBox(-1);
	
	m_shipWidgets = tgui::Group::create();
	window->add(m_shipWidgets, "shipWidgets");

	m_armamentsLabel = tgui::Label::create("Armaments: " + Util::cutOffDecimal(m_weaponsStockpile, 2) + "/100");
	m_armamentsLabel->setPosition("2.5%", "55%");
	window->add(m_armamentsLabel);

	m_buildSpeedLabel = tgui::Label::create("Additional build speed: " + Util::percentify(getBuildSpeedMultiplier(), 2));
	m_buildSpeedLabel->setPosition("2.5%", "60%");
	window->add(m_buildSpeedLabel);
}

void FactoryMod::updateDesigns(Faction* faction) {
	for (DesignerShip ship : faction->getShipDesigns()) {
		bool found = false;
		for (const ShipBuildData& data : m_shipBuildData) {
			if (data.shipName == ship.name) {
				found = true;
				break;
			}
		}

		if (!found) {
			ShipBuildData data(ship.name);
			m_shipBuildData.push_back(data);
		}
	}
}

void FactoryMod::setBuildAll(bool build) {
	for (auto& data : m_shipBuildData) {
		data.build = build;
		data.continuous = build;
	}
}

void FactoryMod::setBuild(const std::string& name, bool build) {
	for (auto& data : m_shipBuildData) {
		if (data.shipName == name) {
			data.build = build;
			data.continuous = build;
			break;
		}
	}
}

float FactoryMod::getBuildSpeedMultiplier() {
	return std::min(100.0f, m_weaponsStockpile) / 100.0f + 1.0f;
}

void FactoryMod::updateDesignsListBox(int selectedIndex) {
	if (m_designsListBox != nullptr) {
		m_designsListBox->removeAllItems();
		for (const ShipBuildData& build : m_shipBuildData) {
			m_designsListBox->addItem(build.shipName);
		}

		if (selectedIndex != -1) {
			m_designsListBox->setSelectedItemByIndex(selectedIndex);
		}
	}
}

FighterBayMod::FighterBayMod(const Unit* unit, Star* star, int allegiance, sf::Color color) {
	for (int i = 0; i < 4; i++) {
		float radius = unit->getCollider().getRadius();
		Spaceship* ship = star->createSpaceship("FIGHTER", unit->getPos() + Random::randVec(-radius, radius), allegiance, color);
		
		if (Random::randBool()) {
			ship->addWeapon(Weapon("LASER_GUN"));
		}
		else {
			ship->addWeapon(Weapon("MACHINE_GUN"));
		}

		ship->disable();
		
		m_fighterShipIds.push_back(ship->getID());
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
		constructNewFighter(currentStar, unit, faction);
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

void FighterBayMod::constructNewFighter(Star* currentStar, Unit* unit, Faction* faction) {
	if (m_ticksToNextFighter == 0) {
		float radius = unit->getCollider().getRadius();
		auto weapons = faction->getWeaponsBelowOrEqualWeaponPoints(1.0);
		
		if (weapons.size() > 0) {
			Spaceship* ship = currentStar->createSpaceship("FIGHTER", unit->getPos() + Random::randVec(-radius, radius), unit->getAllegiance(), unit->getFactionColor());

			int rndIndex = Random::randInt(0, weapons.size() - 1);
			ship->addWeapon(weapons[rndIndex].type);

			if (m_fighterStatus != FIGHTER_STATUS::FIGHTING) {
				ship->disable();
			}

			m_fighterShipIds.push_back(ship->getID());

			m_ticksToNextFighter = 1000;

			DEBUG_PRINT("constructed new fighter");
		}
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

void HabitatMod::interactWithPlanet(Unit* unit, Planet* planet, Star* star) {
	// Transfer passengers to planet
	
	bool firstTime = planet->getColony().getPopulation() == 0;
	planet->getColony().addPopulation(m_population);
	if (firstTime && m_population > 0) {
		planet->getColony().setAllegiance(unit->getAllegiance());
		planet->getColony().setFactionColor(unit->getFactionColor());
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
	Spaceship* bus = currentStar->createSpaceship("SPACE_BUS", pos, allegiance, color);
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

void TradeMod::addItem(const std::string& item, float num, float price) {
	if (m_goods.count(item) == 0) {
		m_goods[item] = num;
	}
	else {
		m_goods[item] += num;
	}
	m_money -= num * price;
}

void TradeMod::interactWithPlanet(Unit* unit, Planet* planet, Star* star) {
	if (m_goods.size() == 0) {
		// Returning journey
		if (m_money > 0.0f) {
			planet->getColony().addWealth(m_money / 10000.0f);
		}
		else {
			planet->getColony().removeWealth(-m_money / 10000.0f);
		}
		m_money = 0.0f;
		Sounds::playSoundLocal("data/sound/money.wav", star, unit->getPos(), 25.0f, 1.0f + Random::randFloat(-0.5f, 0.5f));
	}
	else {
		for (auto& item : m_goods) {
			float revenue = planet->getColony().getTradeGoods().sellGoods(item.first, item.second);
			m_money += revenue;
			planet->getColony().addWealth(revenue / 10000.0f);
		}
		m_goods.clear();
		Sounds::playSoundLocal("data/sound/cargo.wav", star, unit->getPos(), 25.0f, 1.0f + Random::randFloat(-0.5f, 0.5f));
	}
}

void ScienceMod::update(Unit* unit, Star* currentStar, Faction* faction) {
	if (!isEnabled()) return;
	if (faction == nullptr) return;
	faction->addResearchPoints(m_research);
}

void PirateBaseMod::update(Unit* unit, Star* currentStar, Faction* faction) {
	if (m_stolenDesigns.size() > 0) {
		if (m_nextShipPercent >= 100.0f) {
			Spaceship* ship = currentStar->createSpaceship(m_stolenDesigns.front().chassis.type, unit->getPos(), -1, Faction::neutralColor);
			for (const DesignerWeapon& weapon : m_stolenDesigns.front().weapons) {
				ship->addWeapon(weapon.type);
			}
			ship->setPirate(true);

			sf::Vector2f randVel = Random::randVec(-50.0f, 50.0f);
			ship->addVelocity(randVel);
			
			// Move to end of queue
			m_stolenDesigns.push_back(m_stolenDesigns.front());
			m_stolenDesigns.pop_front();

			m_nextShipPercent = 0.0f;
		}
		else {
			m_nextShipPercent += 0.025f / m_stolenDesigns.front().chassis.buildTimeMultiplier;
		}
	}

	if ((m_lifetimeTicks + 1) % 20000 == 0) {
		// Send out a ship to create a new pirate base
		Spaceship* ship = currentStar->createSpaceship("BIG_SPACE_TRUCK", unit->getPos(), -1, Faction::neutralColor);
		ship->setPirate(true);
		ship->addOrder(TravelOrder(currentStar->getConnectedStars()[Random::randInt(0, currentStar->getConnectedStars().size() - 1)]));
		ship->addOrder(EstablishPirateBaseOrder(currentStar->getRandomLocalPos(-10000.0f, 10000.0f), m_theftAllegiance, m_stolenDesigns));
		ship->addOrder(DieOrder());
	}

	m_lifetimeTicks++;
}

void PirateBaseMod::findTheftAllegiance(Star* currentStar, Constellation* constellation) {
	// Find the nearest faction

	if (currentStar->getAllegiance() != -1) {
		m_theftAllegiance = currentStar->getAllegiance();
		return;
	}

	std::vector<Faction>& factions = constellation->getFactions();

	int nearestAllegiance = -1;
	int closestDist = 0;

	for (Faction& faction : factions) {
		if (!faction.isDead()) {
			int dist = Pathfinder::findPath(currentStar, faction.getCapital()).size();
			if (dist < closestDist || closestDist == 0) {
				closestDist = dist;
				nearestAllegiance = faction.getID();
			}
		}
	}

	if (nearestAllegiance != -1) {
		m_theftAllegiance = nearestAllegiance;
	}
}

void PirateBaseMod::stealDesignFrom(Faction* faction) {
	if (faction == nullptr) return;

	auto designs = faction->getShipDesigns();
	std::vector<DesignerShip> viableDesigns;

	for (auto& shipDesign : designs) {
		if (shipDesign.chassis.type != "CONSTRUCTION_SHIP") {
			if (!hasDesign(shipDesign)) {
				viableDesigns.push_back(shipDesign);
			}
		}
	}

	if (viableDesigns.size() > 0) {
		int rnd = Random::randInt(0, viableDesigns.size() - 1);
		m_stolenDesigns.push_back(viableDesigns[rnd]);
		DEBUG_PRINT("Pirates stole the " << m_stolenDesigns.back().name << " design from " << faction->getName());
	}
}

bool PirateBaseMod::hasDesign(const DesignerShip& design) {
	for (auto& d : m_stolenDesigns) {
		if (d.name == design.name) return true;
	}
	return false;
}
