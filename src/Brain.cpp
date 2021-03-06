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
#include "AllianceList.h"

void SubAI::sleep(uint32_t ticks) {
	m_sleepTime = ticks;
}

bool SubAI::sleepCheck() {
	if (m_sleepTime == 0) return true;
	else m_sleepTime--; 
	return false;
}

void Brain::onStart(Faction& faction) {
	for (Planet& planet : faction.getCapital()->getPlanets()) {
		if (planet.getHabitability() > 0.25f) {
			planet.getColony().setFactionColonyLegality(faction.getID(), true);
			AI_DEBUG_PRINT("Made colonization of " << planet.getTypeString() << " legal");
		}
	}

	onResearchComplete(faction);
}

void Brain::controlFaction(Faction& faction, const AllianceList& alliances) {
	controlSubAI(faction, militaryAI, alliances);
	controlSubAI(faction, defenseAI, alliances);
	controlSubAI(faction, economyAI, alliances);
}

void Brain::controlSubAI(Faction& faction, SubAI& subAI, const AllianceList& alliances) {
	if (subAI.sleepCheck()) subAI.update(faction, *this, alliances);
}

void Brain::onColonization(Faction& faction, Planet& planet) {
	planet.getColony().buyBuilding(ColonyBuilding("BASIC_MINING"), &faction, planet);
}

void Brain::onStarTakeover(Faction& faction, Star& star) {
	std::vector<Planet*> toBeColonized;
	
	for (Planet& planet : star.getPlanets()) {

		if (!planet.getColony().isColonizationLegal(faction.getID())) {
			if (planet.getHabitability() > 0.33f) {
				toBeColonized.push_back(&planet);
			}
			else if (planet.getWater() > 0.0f) {
				toBeColonized.push_back(&planet);
			}
		}
	}

	for (Planet* planet : toBeColonized) {
		planet->getColony().setFactionColonyLegality(faction.getID(), true);
		AI_DEBUG_PRINT("Made colonization of " << planet->getTypeString() << " legal");
	}

	// Send a ship to plunder each derelict
	if (star.getDerelicts().size() > 0) {
		std::vector<Spaceship*> ships = star.getAllShipsOfAllegiance(faction.getID());
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

			for (Derelict& derelict : star.getDerelicts()) {
				ship->addOrder(FlyToOrder(derelict.getPos()));
			}
		}
	}
}

void Brain::reinitAfterLoad(Constellation* constellation) {
	militaryAI.reinitAfterLoad(constellation);
}

void Brain::onResearchComplete(Faction& faction) {
	if (economyAI.researchStarterTechs(faction)) {
		economyAI.researchRandomTech(faction);
	}
}

void MilitaryAI::update(Faction& faction, Brain& brain, const AllianceList& alliances) {
	if (faction.getAllCombatShips().size() == 0) {
		sleep(100);
		return;
	}

	// Change states
	if (m_stateChangeTimer == 0) {
		if (Random::randBool() && faction.getAllCombatShips().size() > 6) {
			m_state = MilitaryState::ATTACKING;
			AI_DEBUG_PRINT("Changed state to attacking");
		}
		else {
			m_state = MilitaryState::RALLYING;
			AI_DEBUG_PRINT("Changed state to rallying");
		}
		m_stateChangeTimer = Random::randInt(5000, 15000);
	}
	else {
		m_stateChangeTimer--;
	}

	if (m_state == MilitaryState::ATTACKING) {
		if (m_expansionTarget == nullptr) {
			// Choose expansion target
			
			std::vector<Star*> borderStars = faction.getBorderStars();

			// Prioritize undiscovered stars first
			for (Star* star : borderStars) {
				if (!star->isDiscovered(faction.getID())) {
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
				
				// Don't attack allied stars
				borderStars.erase(std::remove_if(borderStars.begin(), borderStars.end(), [&faction, &alliances](const Star* star) {
					return alliances.isAllied(faction.getID(), star->getAllegiance());
				}), borderStars.end());

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
				faction.giveAllCombatShipsOrder(TravelOrder(m_expansionTarget), true);
				m_launchingAttack = true;
				m_attackTimer = 1600;
				//AI_DEBUG_PRINT("Begun attack");
			}
			else {
				if (m_attackTimer == 0) {
					if (alliances.isAllied(faction.getID(), m_expansionTarget->getAllegiance())) {
						m_expansionTarget = nullptr;
						m_expansionTargetID = 0;
						m_launchingAttack = false;
						//AI_DEBUG_PRINT("Capture of star complete");
					}
					else {
						m_attackTimer = 1600;

						if (m_expansionTarget->numAlliedShips(faction.getID()) == 0) {
							m_attackFrustration++;
						}
						else {
							if (m_attackTimer % 400 == 0) {
								// Micromanage the battle
								std::vector<Spaceship*> alliedShips = m_expansionTarget->getAllShipsOfAllegiance(faction.getID());
								std::vector<Spaceship*> enemyShips = m_expansionTarget->getEnemyCombatShips(faction.getID(), alliances);

								if (alliedShips.size() > enemyShips.size() * 2) {
									// Take out any outposts
									std::vector<Building*> outposts = m_expansionTarget->getBuildingsOfType("OUTPOST");
									for (Building* outpost : outposts) {
										if (!alliances.isAllied(outpost->getAllegiance(), faction.getID())) {
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
			std::vector<Star*> underAttackStars = faction.getUnderAttackStars(alliances);

			if (underAttackStars.size() == 0) {
				// Send an order to all combat ships to travel to the most recently conquered star
				if (faction.getOwnedStars().size() > 0) {
					for (Spaceship* ship : faction.getAllCombatShips()) {
						ship->clearOrders();
						ship->addOrder(TravelOrder(faction.getOwnedStars().back()));
					}
				}
			}
			else {
				// Defend the nation
				for (Spaceship* ship : faction.getAllCombatShips()) {
					ship->clearOrders();
					ship->addOrder(TravelOrder(underAttackStars.front()));
				}
			}

			// Send planet attack ships to attack enemy planets
			for (Star* star : faction.getOwnedStars()) {
				std::vector<Planet*> enemyPlanets = star->getEnemyPlanets(faction.getID(), alliances);
				if (enemyPlanets.size() > 0) {
					std::vector<Spaceship*> planetAttackShips = faction.getPlanetAttackShips();

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

void DefenseAI::update(Faction& faction, Brain& brain, const AllianceList& alliances) {
	if (m_fortifyingTimer == 0) {
		for (Star* star : faction.getOwnedStars()) {
			if (Random::randBool()) {
				if (!star->containsBuildingType("OUTPOST", true, faction.getID()) && faction.numIdleConstructionShips() > 0) {
					// Build outpost

					Building* building = star->createBuilding("OUTPOST", star->getRandomLocalPos(-10000.0f, 10000.0f), &faction, false);

					// Give idle construction ships order to build it
					faction.orderConstructionShipsBuild(building, true);

					AI_DEBUG_PRINT("Building outpost");
				}
			}
			else {
				for (auto& building : star->getBuildings()) {
					// Build any unbuilt buildings
					if (building->getAllegiance() == faction.getID() && !building->isBuilt()) {
						faction.orderConstructionShipsBuild(building.get(), true, true);
						//AI_DEBUG_PRINT("Ordering build of unbuilt building");
					}
				}

				if (Random::randBool() && faction.numIdleConstructionShips() > 0) {
					// Build turrets around jump points

					std::vector<JumpPoint>& jumpPoints = star->getJumpPoints();

					int numTurrets = Random::randInt(0, faction.numIdleConstructionShips() - 1);
					int randJumpPointIndex = Random::randInt(0, jumpPoints.size() - 1);

					JumpPoint& point = jumpPoints[randJumpPointIndex];

					// Get idle construction ships
					std::vector<Spaceship*> conShips = faction.getConstructionShips(true);

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
							if (BuildingPrototype::meetsDisplayRequirements(turr, &faction)) {
								allowedTurrets.push_back(turr);
							}
						}

						if (allowedTurrets.size() > 0) {
							int rndIndex = Random::randInt(0, allowedTurrets.size() - 1);

							// Create turret
							turret = star->createBuilding(allowedTurrets[rndIndex], pos, &faction, false);

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

void EconomyAI::update(Faction& faction, Brain& brain, const AllianceList& alliances) {
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
	
	handleStars(faction);
	handleShips(faction);
	handleShipDesigns(faction);
	handleColonies(faction);
	handleAsteroidMining(faction);

	sleep(1000);
}

void EconomyAI::handleColonies(Faction & faction) {
	// Colonies
	auto planets = faction.getOwnedPlanets();
	if (m_state == EconomyState::DEVELOPING_PLANETS) {
		
		// Sort so that planets with the most habitability are operated on first.
		std::sort(planets.begin(), planets.end(), [](const Planet* p1, const Planet* p2) {
			return p1->getHabitability() > p2->getHabitability();
		});

		for (int i = 0; i < planets.size(); i++) {
			if (!buildColonyBuilding(*planets[i], faction)) break;
		}
	}

	// Explore anomalies on planets
	for (Planet* planet : planets) {
		auto anomalyTiles = planet->getColony().getAnomalyTiles();
		const Colony::Tile& mostPopulatedTile = planet->getColony().getTile(planet->getColony().getMostPopulatedTile());

		// Remove tiles which already have an expedition incoming
		for (int i = 0; i < anomalyTiles.size(); i++) {
			if (planet->getColony().hasExpeditionToTile(anomalyTiles[i])) {
				anomalyTiles.erase(anomalyTiles.begin() + i);
				i--;
			}
		}

		if (anomalyTiles.size() > 0 && mostPopulatedTile.population > 500) {
			int rndIndex = Random::randInt(0, anomalyTiles.size() - 1);
			planet->getColony().sendExpedition(anomalyTiles[rndIndex]);
			AI_DEBUG_PRINT("Sending out an expedition");
		}
	}
}

void EconomyAI::handleShipDesigns(Faction & faction) {
	// Handle ship designs

	// Weapons
	for (auto& weapon : faction.getWeapons()) {
		bool notUsed = true;
		for (auto& design : faction.getShipDesigns()) {
			for (auto& w : design.weapons) {
				if (w.type == weapon.type) {
					notUsed = false;
				}
			}
		}

		// If there doesn't exist a design that uses this weapon, create a new design
		if (notUsed && !weapon.constructionWeapon && !weapon.miningWeapon) {
			DesignerShip newDesign;
			std::vector<DesignerChassis> usableChassis;

			for (auto& chassis : faction.getChassis()) {
				if (chassis.maxWeaponCapacity > 0.0f && !chassis.constructionChassis && !chassis.miningChassis) {
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
				faction.addShipDesign(newDesign);

				AI_DEBUG_PRINT("Created new ship design " << newDesign.name);
			}
		}
	}

	// Chassis
	for (auto& chassis : faction.getChassis()) {
		if (chassis.constructionChassis || chassis.miningChassis || chassis.maxWeaponCapacity == 0.0f) {
			continue;
		}

		bool notUsed = true;
		for (auto& design : faction.getShipDesigns()) {
			if (design.chassis.name == chassis.name) {
				notUsed = false;
			}
		}

		if (notUsed) {
			DesignerShip newDesign;
			newDesign.chassis = chassis;

			std::vector<DesignerWeapon> usableWeapons = faction.getWeaponsBelowOrEqualWeaponPoints(chassis.maxWeaponCapacity);

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
					faction.addShipDesign(newDesign);

					AI_DEBUG_PRINT("Created new ship design " << newDesign.name);
				}
			}
		}
	}
}

void EconomyAI::handleShips(Faction & faction) {
	// Handle building ships
	bool buildShips = false;
	if (m_state == EconomyState::BUILDING_SHIPS) {
		buildShips = true;
	}

	int numConstructors = faction.getConstructionShips().size();
	int numStars = faction.getOwnedStars().size();

	for (Building* factory : faction.getAllOwnedBuildingsOfType("SHIP_FACTORY")) {
		FactoryMod* mod = factory->getMod<FactoryMod>();
		mod->updateDesigns(&faction);

		if (buildShips) {
			for (const DesignerShip& design : faction.getShipDesigns()) {
				if (design.name != "Constructor") {
					mod->setBuild(design.name, true);
				}
				else if (numConstructors < numStars * 2){
					// Limit the amount of construction ships to 2 per star
					mod->setBuild(design.name, true);
				}
			}
		}
		else {
			mod->setBuildAll(false);
		}
	}
}

void EconomyAI::handleStars(Faction & faction) {
	
	// Find the most abundant resource
	auto mostResource = faction.getMostAbundantResource();

	for (Star* star : faction.getOwnedStars()) {

		// Build science labs
		if (star->numAlliedBuildings(faction.getID(), "SCIENCE_LAB") < faction.getScienceLabMax(star) && faction.numIdleConstructionShips() > 0) {
			int numToBuild = faction.getScienceLabMax(star) - star->numAlliedBuildings(faction.getID(), "SCIENCE_LAB");
			for (int i = 0; i < numToBuild && faction.numIdleConstructionShips() > 0; i++) {
				Building* lab = star->createBuilding("SCIENCE_LAB", star->getRandomLocalPos(-10000.0f, 10000.0f), &faction, false);
				faction.orderConstructionShipsBuild(lab, true, true);

				AI_DEBUG_PRINT("Building science lab");
			}
		}

		bool builtShipFactory = false;

		// Build ship factories
		if (star->numAlliedBuildings(faction.getID(), "SHIP_FACTORY") < 1 && faction.numIdleConstructionShips() > 0 &&
			!builtShipFactory) {
			Building* factory = star->createBuilding("SHIP_FACTORY", star->getRandomLocalPos(-10000.0f, 10000.0f), &faction, false);

			faction.orderConstructionShipsBuild(factory, true);

			AI_DEBUG_PRINT("Building ship factory");
			builtShipFactory = true;
		}

		// Manage science labs
		for (Building* lab : faction.getAllOwnedBuildingsOfType("SCIENCE_LAB")) {
			ScienceMod* scienceMod = lab->getMod<ScienceMod>();

			if (mostResource.second < 50.0f) {
				if (scienceMod->isResearching()) {

					scienceMod->setResearching(false);
					AI_DEBUG_PRINT("Turned off a science lab due to not enough resources");
				}
			}
			else {
				scienceMod->setResearching(true);

				if (scienceMod->getResourceType() != mostResource.first) {
					scienceMod->setResourceType(mostResource.first);
					AI_DEBUG_PRINT("Set science lab resource to " << mostResource.first);
				}
			}
		}
	}
}

void removeBuildings(Planet& planet, std::vector<std::string>& wantedBuildings, Faction* faction) {
	for (int i = 0; i < wantedBuildings.size(); i++) {
		ColonyBuilding building(wantedBuildings[i]);
		bool remove = false;

		if (!faction->hasColonyBuilding(building.getType()) || !building.isBuildable(planet.getColony())) {
			remove = true;
		}

		if (building.getType() == "WATER_PUMP" && planet.getWater() == 0.0) {
			remove = true;
		}

		if (remove) {
			wantedBuildings.erase(wantedBuildings.begin() + i);
			i--;
		}
	}
}

bool EconomyAI::buildColonyBuilding(Planet& planet, Faction& faction) {
	std::vector<std::string> wantedBuildings = {
				"FARMING",
				"BASIC_MINING",
				"WATER_PUMP",
				"INFRASTRUCTURE"
	};

	if (planet.hasResource("UNCOMMON_ORE")) {
		wantedBuildings.push_back("CONSUMER_GOODS_FACTORIES");
	}

	std::vector<std::string> lowPriorityBuildings = {
		"SPACEPORT",
		"ORBITAL_DEFENSE",
		"BOMB_SHELTER",
		"MILITARY_BASE",
		"WEAPONS_FACTORIES",
		"FACTORY",
		"LUXURY_GOODS_FACTORY",
		"LUXURY_GOODS_FACTORY_2",
		"MINING",
		"IMPROVED_INFRASTRUCTURE",
		"ADVANCED_INFRASTRUCTURE",
		"CONSUMER_GOODS_FACTORIES_2",
		"CONSUMER_GOODS_FACTORIES_3"
	};

	removeBuildings(planet, wantedBuildings, &faction);
	if (wantedBuildings.size() == 0 ) wantedBuildings = lowPriorityBuildings;
	removeBuildings(planet, wantedBuildings, &faction);

	if (wantedBuildings.size() != 0) {
		int rnd = Random::randInt(0, wantedBuildings.size() - 1);

		ColonyBuilding toBuild(wantedBuildings[rnd]);
		if (toBuild.isGlobal()) {
			toBuild.setPos({ -1, -1 });
		}
		else {
			toBuild.setPos(decideColonyBuildingPlacement(toBuild.getType(), planet.getColony()));

			if (toBuild.getPos() == sf::Vector2i{-1, -1}) {
				return false;
			}
		}

		if (planet.getColony().buyBuilding(toBuild, &faction, planet)) {
			AI_DEBUG_PRINT("Building colony building " << toBuild.getName());
		}
		return false;
	}
	else {
		// All wanted buildings built
		return true;
	}
}

sf::Vector2i EconomyAI::decideColonyBuildingPlacement(const ColonyBuilding& building, const Colony& colony) {
	if (building.getType() == "FARMING") {
		sf::Vector2i mostPopulated = colony.getMostPopulatedTile();
		auto adjacent = colony.getAdjacentTiles(mostPopulated);
		if (adjacent.size() > 0) {
			int rnd = Random::randInt(0, adjacent.size() - 1);
			return adjacent[rnd];
		}
	}

	if (building.getType() == "BASIC_MINING" || building.getType() == "MINING") {
		auto resourceTiles = colony.getResourceTiles();
		if (resourceTiles.size() > 0) {
			int rnd = Random::randInt(0, resourceTiles.size() - 1);
			return resourceTiles[rnd];
		}
	}

	// Find tiles with no building on them
	std::vector<sf::Vector2i> rndTiles;
	for (int y = 0; y < Colony::GRID_LENGTH; y++) {
		for (int x = 0; x < Colony::GRID_LENGTH; x++) {
			if (building.isBuildableOnTile(colony, {x, y})) {
				rndTiles.push_back({ x, y });
			}
		}
	}

	if (rndTiles.size() > 0) {
		int rnd = Random::randInt(0, rndTiles.size() - 1);
		return rndTiles[rnd];
	}

	return { -1, -1 };
}

void EconomyAI::researchRandomTech(Faction& faction) {
	std::vector<Tech> unresearched = faction.getUnresearchedTechs();

	// Autogenerated techs have a lesser priority
	unresearched.erase(std::remove_if(unresearched.begin(), unresearched.end(), [](const Tech& tech) {
		return tech.isAutogenerated();
	}), unresearched.end());

	if (unresearched.size() == 0) {
		unresearched = faction.getUnresearchedTechs();
	}

	if (unresearched.size() > 0) {
		std::string type = unresearched[Random::randInt(0, unresearched.size() - 1)].getType();
		faction.setResearchingTech(type, true);
		AI_DEBUG_PRINT("Started researching " << type);
	}
}

bool EconomyAI::researchStarterTechs(Faction& faction) {
	std::vector<std::string> starterTechs = {
		"MANUFACTURING"
	};
	
	for (int i = 0; i < starterTechs.size(); i++) {
		if (faction.hasResearchedTech(starterTechs[i])) {
			starterTechs.erase(starterTechs.begin() + i);
			i--;
		}
	}

	if (starterTechs.size() == 0) return true;
	else {
		std::string tech = starterTechs[Random::randInt(0, starterTechs.size() - 1)];
		faction.setResearchingTech(tech, true);
		AI_DEBUG_PRINT("Started researching " << tech);
		return false;
	}
}

void EconomyAI::handleAsteroidMining(Faction& faction) {
	for (auto& ship : faction.getShips()) {
		if (ship->hasWeapon("MINING_LASER") && ship->numOrders() == 0) {
			Star* targetStar = ship->getCurrentStar();
			auto& ownedStars = faction.getOwnedStars();
			int ownedStarsIndex = -1;
			bool doneSearching = false;

			// Search current stars first for asteroids, then search other stars
			do {
				std::deque<Asteroid*> miningQueue;

				for (auto& asteroid : targetStar->getAsteroids()) {
					if (!asteroid->isDestructing()) {
						if (asteroid->getResource() == "COMMON_ORE") {
							miningQueue.push_front(asteroid.get());
						}
						else {
							miningQueue.push_back(asteroid.get());
						}
					}
				}

				if (miningQueue.size() != 0) {
					if (ship->getCurrentStar() != targetStar) {
						ship->addOrder<TravelOrder>(targetStar);
					}
					ship->addOrder<MineAsteroidOrder>(MineAsteroidOrder(miningQueue.front()));
					doneSearching = true;
					AI_DEBUG_PRINT("Sent a ship to go mining");
				}
				else {
					ownedStarsIndex++;
					if (ownedStarsIndex == ownedStars.size()) {
						doneSearching = true;
					}
					else {
						targetStar = ownedStars[ownedStarsIndex];
					}
				}
			} while (!doneSearching);
		}
	}
}