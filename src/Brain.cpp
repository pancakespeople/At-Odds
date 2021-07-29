#include "gamepch.h"
#include "Brain.h"
#include "Faction.h"
#include "Spaceship.h"
#include "Order.h"
#include "Constellation.h"
#include "Random.h"
#include "Building.h"
#include "Derelict.h"

void Brain::onStart(Faction* faction) {
	
}

void Brain::controlFaction(Faction* faction) {
	if (m_state == AI_STATE::NONE || m_stateChangeTimer <= 0) {
		considerChangingState();
	}
	else if (m_state == AI_STATE::ATTACKING) {
		considerAttack(faction);
	}
	else if (m_state == AI_STATE::FORTIFYING) {
		considerFortifying(faction);
	}
	else if (m_state == AI_STATE::ECONOMY) {
		considerEconomy(faction);
	}

	m_stateChangeTimer--;
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

void Brain::considerChangingState() {
	if (Random::randFloat(0.0f, 1.0f) < m_personality.aggressiveness) {
		m_state = AI_STATE::ATTACKING;
	}
	else {
		if (Random::randFloat(0.0f, 1.0f) < m_personality.economizer) {
			m_state = AI_STATE::ECONOMY;
		}
		else {
			m_state = AI_STATE::FORTIFYING;
		}
	}

	m_stateChangeTimer = 3000;
}

void Brain::onSpawn(Faction* faction) {
	for (Planet& planet : faction->getCapitol()->getPlanets()) {
		if (planet.getHabitability() > 0.5f || planet.getResources().size() > 0) {
			planet.getColony().setFactionColonyLegality(faction->getID(), true);
			AI_DEBUG_PRINT("Made colonization of " << planet.getTypeString() << " legal");
		}
	}
}

void Brain::considerFortifying(Faction* faction) {
	if (m_fortifyingVars.fortifyingTimer == 0) {
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

					int numTurrets = Random::randInt(0, 4);
					int randJumpPointIndex = Random::randInt(0, jumpPoints.size() - 1);

					JumpPoint& point = jumpPoints[randJumpPointIndex];

					// Pick the first idle construction ship found to build the turrets
					Spaceship* conShip = faction->getConstructionShips(true)[0];

					for (int i = 0; i < numTurrets; i++) {
						sf::Vector2f pos = point.getPos() + Random::randVec(-2500.0f, 2500.0f);
						int rnd = Random::randInt(0, 4);

						Building* turret = nullptr;

						const std::vector<std::string> turrets = {
							"LASER_TURRET",
							"MACHINE_GUN_TURRET",
							"GAUSS_TURRET"
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
							conShip->addOrder(InteractWithBuildingOrder(turret));
						}
					}

					AI_DEBUG_PRINT("Building " << numTurrets << " turrets");
				}
			}
		}
		m_fortifyingVars.fortifyingTimer = 500;
	}

	m_fortifyingVars.fortifyingTimer--;
}

void Brain::considerAttack(Faction* faction) {
	if (faction->getAllCombatShips().size() == 0) {
		m_state = AI_STATE::NONE;
		return;
	}
	
	if (m_attackVars.expansionTarget == nullptr) {

		// Secure stars connected to capitol
		for (Star* s : faction->getCapitol()->getConnectedStars()) {
			if (s->getAllegiance() != faction->getID()) {
				m_attackVars.expansionTarget = s;
				//AI_DEBUG_PRINT("Expansion target (near capitol) chosen");
				break;
			}
		}

		if (m_attackVars.expansionTarget == nullptr) {
			std::vector<Star*>& ownedStars = faction->getOwnedStars();
			for (int i = faction->getOwnedStars().size() - 1; i > 0; i--) {
				for (Star* adj : ownedStars[i]->getConnectedStars()) {
					if (adj->getAllegiance() != faction->getID()) {
						m_attackVars.expansionTarget = adj;
						//AI_DEBUG_PRINT("Expansion target chosen");
						break;
					}
				}
				if (m_attackVars.expansionTarget != nullptr) {
					break;
				}
			}
		}

	}
	else {
		if (!m_attackVars.launchingAttack) {
			faction->giveAllCombatShipsOrder(TravelOrder(m_attackVars.expansionTarget));
			m_attackVars.launchingAttack = true;
			m_attackVars.attackTimer = 1600;
			//AI_DEBUG_PRINT("Begun attack");
		}
		else {
			if (m_attackVars.attackTimer == 0) {
				if (m_attackVars.expansionTarget->getAllegiance() == faction->getID()) {
					m_attackVars.expansionTarget = nullptr;
					m_attackVars.launchingAttack = false;
					//AI_DEBUG_PRINT("Capture of star complete");
				}
				else {
					m_attackVars.attackTimer = 1600;

					if (m_attackVars.expansionTarget->numAlliedShips(faction->getID()) == 0) {
						m_attackVars.attackFrustration++;
					}
					else {
						m_attackVars.attackFrustration = 0;
					}

					if (m_attackVars.attackFrustration >= 5) {
						m_attackVars.expansionTarget = nullptr;
						m_attackVars.launchingAttack = false;
						m_attackVars.attackFrustration = 0;
						//AI_DEBUG_PRINT("Gave up on capture of star");
					}
				}
			}
			else {
				m_attackVars.attackTimer--;
			}
		}
	}
}

void Brain::considerEconomy(Faction* faction) {
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
			if (!mostHabitable.getColony().isColonizationLegal(faction->getID())) {
				mostHabitable.getColony().setFactionColonyLegality(faction->getID(), true);
				AI_DEBUG_PRINT("Made colonization of " << mostHabitable.getTypeString() << " legal");
			}

			// 50% to set another random other planet to legal
			if (Random::randBool()) {
				auto planets = star->getPlanets();
				Planet& randPlanet = planets[Random::randInt(0, planets.size() - 1)];

				if (!randPlanet.getColony().isColonizationLegal(faction->getID())) {
					randPlanet.getColony().setFactionColonyLegality(faction->getID(), true);
					AI_DEBUG_PRINT("Made colonization of " << randPlanet.getTypeString() << " legal");
				}
			}
		}
	}

	// Save up resources or spend them
	if (Random::randBool() || faction->getAllCombatShips().size() == 0) {
			
		for (Building* factory : faction->getAllOwnedBuildingsOfName("Ship Factory")) {
			FactoryMod* mod = factory->getMod<FactoryMod>();
			mod->updateDesigns(faction);
			mod->setBuildAll(true);
		}

		AI_DEBUG_PRINT("Decided to spend resources");
	}
	else {
		for (Building* factory : faction->getAllOwnedBuildingsOfName("Ship Factory")) {
			FactoryMod* mod = factory->getMod<FactoryMod>();
			mod->updateDesigns(faction);
			mod->setBuildAll(false);
		}

		AI_DEBUG_PRINT("Decided to save resources");
	}

	// Handle ship designs
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

			newDesign.name = newDesign.generateName();
			faction->addOrReplaceDesignerShip(newDesign);

			AI_DEBUG_PRINT("Created new ship design " << newDesign.name);
		}
	}

	m_state = AI_STATE::NONE;
}