#include "gamepch.h"
#include "Brain.h"
#include "Faction.h"
#include "Spaceship.h"
#include "Order.h"
#include "Constellation.h"
#include "Random.h"
#include "Building.h"
#include "Derelict.h"

void SubAI::sleep(uint32_t ticks) {
	m_sleepTime = ticks;
}

bool SubAI::sleepCheck() {
	if (m_sleepTime == 0) return true;
	else m_sleepTime--; 
	return false;
}

void Brain::onStart(Faction* faction) {
	
}

void Brain::controlFaction(Faction* faction) {
	controlSubAI(faction, &militaryAI);
	controlSubAI(faction, &defenseAI);
	controlSubAI(faction, &economyAI);
}

void Brain::controlSubAI(Faction* faction, SubAI* subAI) {
	if (subAI->sleepCheck()) subAI->update(faction, this);
}

void Brain::onStarTakeover(Faction* faction, Star* star) {
	for (Planet& planet : star->getPlanets()) {
		if (planet.getHabitability() > 0.5f || planet.getResources().size() > 0) {
			planet.getColony().setFactionColonyLegality(faction->getID(), true);
			AI_DEBUG_PRINT("Made colonization of " << planet.getTypeString() << " legal");
		}
	}

	// Send a ship to plunder each derelict
	if (star->getDerelicts().size() > 0) {
		std::vector<Spaceship*> ships = star->getAllShipsOfAllegiance(faction->getID());
		ships.erase(std::remove_if(ships.begin(), ships.end(), [](Spaceship* s) {
			if (s->isCivilian() || s->isDead() || s->isDisabled() || !s->canReceiveOrders() ||
				!s->canPlayerGiveOrders()) {
				return true;
			}
			return false;
			}), ships.end());

		if (ships.size() > 0) {
			int rndShip = Random::randInt(0, ships.size() - 1);
			Spaceship* ship = ships[rndShip];
			ship->clearOrders();

			AI_DEBUG_PRINT("Sending a ship to plunder a derelict");

			for (Derelict& derelict : star->getDerelicts()) {
				ship->addOrder(FlyToOrder(derelict.getPos()));
			}
		}
	}
}

void Brain::onSpawn(Faction* faction) {
	for (Planet& planet : faction->getCapital()->getPlanets()) {
		if (planet.getHabitability() > 0.5f || planet.getResources().size() > 0) {
			planet.getColony().setFactionColonyLegality(faction->getID(), true);
			AI_DEBUG_PRINT("Made colonization of " << planet.getTypeString() << " legal");
		}
	}
}

void MilitaryAI::update(Faction* faction, Brain* brain) {
	if (faction->getAllCombatShips().size() == 0) {
		sleep(100);
		return;
	}

	if (m_expansionTarget == nullptr) {

		// Secure stars connected to capital
		for (Star* s : faction->getCapital()->getConnectedStars()) {
			if (s->getAllegiance() != faction->getID()) {
				m_expansionTarget = s;
				//AI_DEBUG_PRINT("Expansion target (near capital) chosen");
				break;
			}
		}

		if (m_expansionTarget == nullptr) {
			std::vector<Star*>& ownedStars = faction->getOwnedStars();
			for (int i = faction->getOwnedStars().size() - 1; i > 0; i--) {
				for (Star* adj : ownedStars[i]->getConnectedStars()) {
					if (adj->getAllegiance() != faction->getID()) {
						m_expansionTarget = adj;
						//AI_DEBUG_PRINT("Expansion target chosen");
						break;
					}
				}
				if (m_expansionTarget != nullptr) {
					break;
				}
			}
		}

	}
	else {
		if (!m_launchingAttack) {
			faction->giveAllCombatShipsOrder(TravelOrder(m_expansionTarget));
			m_launchingAttack = true;
			m_attackTimer = 1600;
			//AI_DEBUG_PRINT("Begun attack");
		}
		else {
			if (m_attackTimer == 0) {
				if (m_expansionTarget->getAllegiance() == faction->getID()) {
					m_expansionTarget = nullptr;
					m_launchingAttack = false;
					//AI_DEBUG_PRINT("Capture of star complete");
				}
				else {
					m_attackTimer = 1600;

					if (m_expansionTarget->numAlliedShips(faction->getID()) == 0) {
						m_attackFrustration++;
					}
					else {
						m_attackFrustration = 0;
					}

					if (m_attackFrustration >= 5) {
						m_expansionTarget = nullptr;
						m_launchingAttack = false;
						m_attackFrustration = 0;
						//AI_DEBUG_PRINT("Gave up on capture of star");
					}
				}
			}
			else {
				m_attackTimer--;
			}
		}
	}
}

void DefenseAI::update(Faction* faction, Brain* brain) {
	if (m_fortifyingTimer == 0) {
		for (Star* star : faction->getOwnedStars()) {
			if (Random::randBool()) {
				if (!star->containsBuildingName("Outpost", true, faction->getID()) && faction->numIdleConstructionShips() > 0) {
					// Build outpost

					std::unique_ptr<Building> building = std::make_unique<Building>(
						"OUTPOST", star, star->getRandomLocalPos(-10000.0f, 10000.0f), faction, false);
					Building* realBuilding = star->createBuilding(building);

					// Give idle construction ships order to build it
					faction->orderConstructionShipsBuild(realBuilding, true);

					AI_DEBUG_PRINT("Building outpost");
				}
			}
			else {
				for (auto& building : star->getBuildings()) {
					// Build any unbuilt buildings
					if (building->getAllegiance() == faction->getID() && !building->isBuilt()) {
						faction->orderConstructionShipsBuild(building.get(), true, true);
						//AI_DEBUG_PRINT("Ordering build of unbuilt building");
					}
				}

				if (Random::randBool() && faction->numIdleConstructionShips() > 0) {
					// Build turrets around jump points

					std::vector<JumpPoint>& jumpPoints = star->getJumpPoints();

					int numTurrets = Random::randInt(0, faction->numIdleConstructionShips() - 1);
					int randJumpPointIndex = Random::randInt(0, jumpPoints.size() - 1);

					JumpPoint& point = jumpPoints[randJumpPointIndex];

					// Get idle construction ships
					std::vector<Spaceship*> conShips = faction->getConstructionShips(true);

					for (int i = 0; i < numTurrets; i++) {
						sf::Vector2f pos = point.getPos() + Random::randVec(-2500.0f, 2500.0f);

						Building* turret = nullptr;

						const std::vector<std::string> turrets = {
							"LASER_TURRET",
							"MACHINE_GUN_TURRET",
							"GAUSS_TURRET",
							"ROCKET_TURRET",
							"FLAK_TURRET"
						};

						std::vector<std::string> allowedTurrets;

						for (const std::string& turr : turrets) {
							if (BuildingPrototype::meetsDisplayRequirements(turr, faction)) {
								allowedTurrets.push_back(turr);
							}
						}

						if (allowedTurrets.size() > 0) {
							int rndIndex = Random::randInt(0, allowedTurrets.size() - 1);

							// Create turret
							std::unique_ptr<Building> building = std::make_unique<Building>(allowedTurrets[rndIndex], star, pos, faction, false);
							turret = star->createBuilding(building);

							// Order turret to be built
							conShips[i]->addOrder(InteractWithBuildingOrder(turret));
						}
					}

					AI_DEBUG_PRINT("Building " << numTurrets << " turrets");
				}
			}
		}
		m_fortifyingTimer = 500;
	}

	m_fortifyingTimer--;
}

void EconomyAI::update(Faction* faction, Brain* brain) {
	// Decide economy state
	if (m_stateChangeTimer == 0 || m_state == EconomyState::NONE) {
		if (Random::randBool()) {
			m_state = EconomyState::BUILDING_SHIPS;
			AI_DEBUG_PRINT("Changed state to building ships");
		}
		else {
			m_state = EconomyState::DEVELOPING_PLANETS;
			AI_DEBUG_PRINT("Changed state to developing planets");
		}

		m_stateChangeTimer = Random::randInt(1, 10);
	}
	else {
		m_stateChangeTimer--;
	}
	
	for (Star* star : faction->getOwnedStars()) {

		bool builtShipFactory = false;

		// Build ship factories
		if (!star->containsBuildingName("Ship Factory", true, faction->getID()) && faction->numIdleConstructionShips() > 0 &&
			!builtShipFactory) {
			std::unique_ptr<Building> factory = std::make_unique<Building>(
				"SHIP_FACTORY", star, star->getRandomLocalPos(-10000.0f, 10000.0f), faction, false);

			Building* ptr = star->createBuilding(factory);
			faction->orderConstructionShipsBuild(ptr, true);

			AI_DEBUG_PRINT("Building ship factory");
			builtShipFactory = true;
		}

		// Set colonization of planets to be legal
		if (star->getPlanets().size() > 0) {
			Planet& mostHabitable = star->getMostHabitablePlanet();
			if (!mostHabitable.getColony().isColonizationLegal(faction->getID()) && mostHabitable.getResources().size() > 0) {
				mostHabitable.getColony().setFactionColonyLegality(faction->getID(), true);
				AI_DEBUG_PRINT("Made colonization of " << mostHabitable.getTypeString() << " legal");
			}

			// 50% to set another random other planet to legal
			if (Random::randBool()) {
				auto planets = star->getPlanets();
				Planet& randPlanet = planets[Random::randInt(0, planets.size() - 1)];

				if (!randPlanet.getColony().isColonizationLegal(faction->getID()) && randPlanet.getResources().size() > 0) {
					randPlanet.getColony().setFactionColonyLegality(faction->getID(), true);
					AI_DEBUG_PRINT("Made colonization of " << randPlanet.getTypeString() << " legal");
				}
			}
		}
	}

	// Handle building ships
	bool buildShips = false;
	if (m_state == EconomyState::BUILDING_SHIPS) {
		buildShips = true;
	}
	for (Building* factory : faction->getAllOwnedBuildingsOfName("Ship Factory")) {
		FactoryMod* mod = factory->getMod<FactoryMod>();
		mod->updateDesigns(faction);
		mod->setBuildAll(buildShips);
	}

	// Handle ship designs

	// Weapons
	for (auto& weapon : faction->getWeapons()) {
		bool notUsed = true;
		for (auto& design : faction->getShipDesigns()) {
			for (auto& w : design.weapons) {
				if (w.type == weapon.type) {
					notUsed = false;
				}
			}
		}

		// If there doesn't exist a design that uses this weapon, create a new design
		if (notUsed) {
			Spaceship::DesignerShip newDesign;
			std::vector<Spaceship::DesignerChassis> usableChassis;

			for (auto& chassis : faction->getChassis()) {
				if (chassis.maxWeaponCapacity > 0.0f) {
					usableChassis.push_back(chassis);
				}
			}

			// Select random combat chassis
			int rndChassis = Random::randInt(0, usableChassis.size() - 1);
			newDesign.chassis = usableChassis[rndChassis];

			// Add weapons
			int i = 0;
			const int maxIter = 10;
			while (newDesign.getTotalWeaponPoints() < newDesign.chassis.maxWeaponCapacity && i < maxIter) {
				newDesign.weapons.push_back(weapon);
				i++;
			}

			if (newDesign.getTotalWeaponPoints() > newDesign.chassis.maxWeaponCapacity) {
				newDesign.weapons.pop_back();
			}

			if (newDesign.weapons.size() > 0) {
				newDesign.name = newDesign.generateName();
				faction->addOrReplaceDesignerShip(newDesign);

				AI_DEBUG_PRINT("Created new ship design " << newDesign.name);
			}
		}
	}

	// Chassis
	for (auto& chassis : faction->getChassis()) {
		bool notUsed = true;
		for (auto& design : faction->getShipDesigns()) {
			if (design.chassis.name == chassis.name) {
				notUsed = false;
			}
		}

		if (notUsed) {
			Spaceship::DesignerShip newDesign;
			newDesign.chassis = chassis;

			std::vector<Spaceship::DesignerWeapon> usableWeapons = faction->getWeaponsBelowOrEqualWeaponPoints(chassis.maxWeaponCapacity);

			if (usableWeapons.size() > 0) {
				int i = 0;
				const int maxIter = 10;
				while (newDesign.getTotalWeaponPoints() < newDesign.chassis.maxWeaponCapacity && i < maxIter) {
					int rndIndex = Random::randInt(0, usableWeapons.size() - 1);

					if (newDesign.getTotalWeaponPoints() + usableWeapons[rndIndex].weaponPoints <= chassis.maxWeaponCapacity) {
						newDesign.weapons.push_back(usableWeapons[rndIndex]);
					}

					i++;
				}

				if (newDesign.weapons.size() > 0) {
					newDesign.name = newDesign.generateName();
					faction->addOrReplaceDesignerShip(newDesign);

					AI_DEBUG_PRINT("Created new ship design " << newDesign.name);
				}
			}
		}
	}

	// Colonies
	if (m_state == EconomyState::DEVELOPING_PLANETS) {
		Planet* bestColony = faction->getMostHabitablePlanet();
		bool bestColonyDone = false;
		
		if (bestColony != nullptr) {
			bestColonyDone = buildColonyBuilding(*bestColony, faction);
		}

		if (bestColonyDone) {
			Star* rndStar = faction->getRandomOwnedStar();
			if (rndStar != nullptr) {
				Planet* best = rndStar->getMostHabitablePlanet(faction->getID());
				if (best != nullptr) {
					buildColonyBuilding(*best, faction);
				}
			}
		}
	}

	sleep(1000);
}

bool EconomyAI::buildColonyBuilding(Planet& planet, Faction* faction) {
	std::vector<std::string> wantedBuildings = {
				"FARMING",
				"MINING",
				"SPACEPORT"
	};

	// Check if the wanted buildings are built
	for (int i = 0; i < wantedBuildings.size(); i++) {
		if (planet.getColony().hasBuildingOfType(wantedBuildings[i])) {
			wantedBuildings.erase(wantedBuildings.begin() + i);
			i--;
		}
	}

	if (wantedBuildings.size() != 0) {
		int rnd = Random::randInt(0, wantedBuildings.size() - 1);

		ColonyBuilding toBuild(wantedBuildings[rnd]);
		if (planet.getColony().buyBuilding(toBuild, faction, planet)) {
			AI_DEBUG_PRINT("Building colony building " << toBuild.getName());
		}
		return false;
	}
	else {
		// All wanted buildings built
		return true;
	}
}