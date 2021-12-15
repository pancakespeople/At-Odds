#include "gamepch.h"
#include "Brain.h"
#include "Faction.h"
#include "Spaceship.h"
#include "Order.h"
#include "Constellation.h"
#include "Random.h"
#include "Building.h"
#include "Derelict.h"
#include "Tech.h"

void SubAI::sleep(uint32_t ticks) {
	m_sleepTime = ticks;
}

bool SubAI::sleepCheck() {
	if (m_sleepTime == 0) return true;
	else m_sleepTime--; 
	return false;
}

void Brain::onStart(Faction* faction) {
	for (Planet& planet : faction->getCapital()->getPlanets()) {
		if (planet.getHabitability() > 0.5f || planet.getResources().size() > 0) {
			planet.getColony().setFactionColonyLegality(faction->getID(), true);
			AI_DEBUG_PRINT("Made colonization of " << planet.getTypeString() << " legal");
		}
	}

	onResearchComplete(faction);
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
	std::vector<Planet*> toBeColonized;
	
	for (Planet& planet : star->getPlanets()) {

		if (!planet.getColony().isColonizationLegal(faction->getID())) {
			if (planet.getResources().size() >= 2) {
				toBeColonized.push_back(&planet);
			}
			else if (planet.getHabitability() > 0.5f) {
				toBeColonized.push_back(&planet);
			}
			else if (planet.getWater() > 0.0f) {
				toBeColonized.push_back(&planet);
			}
		}
	}

	for (Planet* planet : toBeColonized) {
		planet->getColony().setFactionColonyLegality(faction->getID(), true);
		AI_DEBUG_PRINT("Made colonization of " << planet->getTypeString() << " legal");
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

void Brain::reinitAfterLoad(Constellation* constellation) {
	militaryAI.reinitAfterLoad(constellation);
}

void Brain::onResearchComplete(Faction* faction) {
	if (economyAI.researchStarterTechs(faction)) {
		economyAI.researchRandomTech(faction);
	}
}

void MilitaryAI::update(Faction* faction, Brain* brain) {
	if (faction->getAllCombatShips().size() == 0) {
		sleep(100);
		return;
	}

	// Change states
	if (m_stateChangeTimer == 0) {
		if (Random::randBool()) {
			m_state = MilitaryState::RALLYING;
			AI_DEBUG_PRINT("Changed state to rallying");
		}
		else {
			m_state = MilitaryState::ATTACKING;
			AI_DEBUG_PRINT("Changed state to attacking");
		}
		m_stateChangeTimer = Random::randInt(5000, 15000);
	}
	else {
		m_stateChangeTimer--;
	}

	if (m_state == MilitaryState::ATTACKING) {
		if (m_expansionTarget == nullptr) {
			// Choose expansion target
			
			std::vector<Star*> borderStars = faction->getBorderStars();

			// Prioritize undiscovered stars first
			for (Star* star : borderStars) {
				if (!star->isDiscovered(faction->getID())) {
					m_expansionTarget = star;
					m_expansionTargetID = star->getID();
					AI_DEBUG_PRINT("Attacking undiscovered star");
					break;
				}
			}

			// Go for neutral stars next
			if (m_expansionTarget == nullptr) {
				for (Star* star : borderStars) {
					if (star->getAllegiance() == -1) {
						m_expansionTarget = star;
						m_expansionTargetID = star->getID();
						AI_DEBUG_PRINT("Attacking neutral star");
						break;
					}
				}
			}

			// Choose random stars next
			if (m_expansionTarget == nullptr) {
				if (borderStars.size() > 0) {
					int rnd = Random::randInt(0, borderStars.size() - 1);
					m_expansionTarget = borderStars[rnd];
					m_expansionTargetID = borderStars[rnd]->getID();
					AI_DEBUG_PRINT("Attacking random star");
				}
			}
		}
		else {
			if (!m_launchingAttack) {
				faction->giveAllCombatShipsOrder(TravelOrder(m_expansionTarget), true);
				m_launchingAttack = true;
				m_attackTimer = 1600;
				//AI_DEBUG_PRINT("Begun attack");
			}
			else {
				if (m_attackTimer == 0) {
					if (m_expansionTarget->getAllegiance() == faction->getID()) {
						m_expansionTarget = nullptr;
						m_expansionTargetID = 0;
						m_launchingAttack = false;
						//AI_DEBUG_PRINT("Capture of star complete");
					}
					else {
						m_attackTimer = 1600;

						if (m_expansionTarget->numAlliedShips(faction->getID()) == 0) {
							m_attackFrustration++;
						}
						else {
							if (m_attackTimer % 400 == 0) {
								// Micromanage the battle
								std::vector<Spaceship*> alliedShips = m_expansionTarget->getAllShipsOfAllegiance(faction->getID());
								std::vector<Spaceship*> enemyShips = m_expansionTarget->getEnemyCombatShips(faction->getID());

								if (alliedShips.size() > enemyShips.size() * 2) {
									// Take out any outposts
									std::vector<Building*> outposts = m_expansionTarget->getBuildingsOfType("OUTPOST");
									for (Building* outpost : outposts) {
										if (outpost->getAllegiance() != faction->getID()) {
											for (Spaceship* ship : alliedShips) {
												ship->clearOrders();
												ship->addOrder(AttackOrder(outpost));
											}
											AI_DEBUG_PRINT("Attacking outpost");
											break;
										}
									}
								}
							}
							
							m_attackFrustration = 0;
						}

						if (m_attackFrustration >= 5) {
							m_expansionTarget = nullptr;
							m_expansionTargetID = 0;
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
	else if (m_state == MilitaryState::RALLYING) {
		if (m_rallyTimer == 0) {
			std::vector<Star*> underAttackStars = faction->getUnderAttackStars();

			if (underAttackStars.size() == 0) {
				// Send an order to all combat ships to travel to the most recently conquered star
				if (faction->getOwnedStars().size() > 0) {
					for (Spaceship* ship : faction->getAllCombatShips()) {
						ship->clearOrders();
						ship->addOrder(TravelOrder(faction->getOwnedStars().back()));
					}
				}
			}
			else {
				// Defend the nation
				for (Spaceship* ship : faction->getAllCombatShips()) {
					ship->clearOrders();
					ship->addOrder(TravelOrder(underAttackStars.front()));
				}
			}

			// Send planet attack ships to attack enemy planets
			for (Star* star : faction->getOwnedStars()) {
				std::vector<Planet*> enemyPlanets = star->getEnemyPlanets(faction->getID());
				if (enemyPlanets.size() > 0) {
					std::vector<Spaceship*> planetAttackShips = faction->getPlanetAttackShips();

					for (Spaceship* ship : planetAttackShips) {
						if (!ship->isHeavy()) {
							ship->clearOrders();
							ship->addOrder(TravelOrder(star));
							ship->addOrder(InteractWithPlanetOrder(enemyPlanets.front(), star));
						}
					}

					if (planetAttackShips.size() > 0) AI_DEBUG_PRINT("Sent " << planetAttackShips.size() << " ships to attack a planet");

					break;
				}
			}
			
			m_rallyTimer = 500;
		}
		else {
			m_rallyTimer--;
		}
	}
}

void MilitaryAI::reinitAfterLoad(Constellation* constellation) {
	m_expansionTarget = constellation->getStarByID(m_expansionTargetID);
}

void DefenseAI::update(Faction* faction, Brain* brain) {
	if (m_fortifyingTimer == 0) {
		for (Star* star : faction->getOwnedStars()) {
			if (Random::randBool()) {
				if (!star->containsBuildingType("OUTPOST", true, faction->getID()) && faction->numIdleConstructionShips() > 0) {
					// Build outpost

					Building* building = star->createBuilding("OUTPOST", star->getRandomLocalPos(-10000.0f, 10000.0f), faction, false);

					// Give idle construction ships order to build it
					faction->orderConstructionShipsBuild(building, true);

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
							turret = star->createBuilding(allowedTurrets[rndIndex], pos, faction, false);

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

		// Build science labs
		if (star->numAlliedBuildings(faction->getID(), "SCIENCE_LAB") < faction->getScienceLabMax(star) && faction->numIdleConstructionShips() > 0) {
			int numToBuild = faction->getScienceLabMax(star) - star->numAlliedBuildings(faction->getID(), "SCIENCE_LAB");
			for (int i = 0; i < numToBuild && faction->numIdleConstructionShips() > 0; i++) {
				Building* lab = star->createBuilding("SCIENCE_LAB", star->getRandomLocalPos(-10000.0f, 10000.0f), faction, false);
				faction->orderConstructionShipsBuild(lab, true, true);

				AI_DEBUG_PRINT("Building science lab");
			}
		}

		bool builtShipFactory = false;

		// Build ship factories
		if (star->numAlliedBuildings(faction->getID(), "SHIP_FACTORY") < 5 && faction->numIdleConstructionShips() > 0 &&
			!builtShipFactory) {
			Building* factory = star->createBuilding("SHIP_FACTORY", star->getRandomLocalPos(-10000.0f, 10000.0f), faction, false);

			faction->orderConstructionShipsBuild(factory, true);

			AI_DEBUG_PRINT("Building ship factory");
			builtShipFactory = true;
		}

	}

	// Handle building ships
	bool buildShips = false;
	if (m_state == EconomyState::BUILDING_SHIPS) {
		buildShips = true;
	}

	for (Building* factory : faction->getAllOwnedBuildingsOfType("SHIP_FACTORY")) {
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
			DesignerShip newDesign;
			std::vector<DesignerChassis> usableChassis;

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
				faction->addShipDesign(newDesign);

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
			DesignerShip newDesign;
			newDesign.chassis = chassis;

			std::vector<DesignerWeapon> usableWeapons = faction->getWeaponsBelowOrEqualWeaponPoints(chassis.maxWeaponCapacity);

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
					faction->addShipDesign(newDesign);

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

void removeBuildings(Planet& planet, std::vector<std::string>& wantedBuildings, Faction* faction) {
	for (int i = 0; i < wantedBuildings.size(); i++) {
		if (planet.getColony().hasBuildingOfType(wantedBuildings[i]) || !faction->hasColonyBuilding(wantedBuildings[i])) {
			wantedBuildings.erase(wantedBuildings.begin() + i);
			i--;
		}
	}
}

bool EconomyAI::buildColonyBuilding(Planet& planet, Faction* faction) {
	std::vector<std::string> wantedBuildings = {
				"FARMING",
				"MINING",
				"SPACEPORT"
	};

	if (planet.hasResource("UNCOMMON_ORE")) {
		wantedBuildings.push_back("CONSUMER_GOODS_FACTORIES");
	}

	std::vector<std::string> lowPriorityBuildings = {
		"EXPLORING",
		"ORBITAL_DEFENSE",
		"BOMB_SHELTER",
		"MILITARY_BASE",
		"WEAPONS_FACTORIES"
	};

	removeBuildings(planet, wantedBuildings, faction);
	if (wantedBuildings.size() == 0 ) wantedBuildings = lowPriorityBuildings;
	removeBuildings(planet, wantedBuildings, faction);

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

void EconomyAI::researchRandomTech(Faction* faction) {
	std::vector<Tech> unresearched = faction->getUnresearchedTechs();
	if (unresearched.size() > 0) {
		std::string type = unresearched[Random::randInt(0, unresearched.size() - 1)].getType();
		faction->setResearchingTech(type, true);
		AI_DEBUG_PRINT("Started researching " << type);
	}
}

bool EconomyAI::researchStarterTechs(Faction* faction) {
	std::vector<std::string> starterTechs = {
		"FARMING",
		"MINING"
	};
	
	for (int i = 0; i < starterTechs.size(); i++) {
		if (faction->hasResearchedTech(starterTechs[i])) {
			starterTechs.erase(starterTechs.begin() + i);
			i--;
		}
	}

	if (starterTechs.size() == 0) return true;
	else {
		std::string tech = starterTechs[Random::randInt(0, starterTechs.size() - 1)];
		faction->setResearchingTech(tech, true);
		AI_DEBUG_PRINT("Started researching " << tech);
		return false;
	}
}