#include "gamepch.h"

#include "Faction.h"
#include "Debug.h"
#include "Constellation.h"
#include "Random.h"
#include "Player.h"
#include "Building.h"
#include "TOMLCache.h"

Faction::Faction(int id) {
	m_color = sf::Color(rand() % 255, rand() % 255, rand() % 255);

	m_id = id;

	if (m_aiEnabled) m_ai.onStart(this);
}

void Faction::spawnAtRandomStar(Constellation* constellation) {
	Star* randStar = constellation->getStars()[Random::randInt(0, constellation->getStars().size() - 1)].get();

	int iterations = 0;
	while (randStar->getAllegiance() != -1 && iterations < 50) {
		randStar = constellation->getStars()[Random::randInt(0, constellation->getStars().size() - 1)].get();
		iterations++;
	}

	randStar->destroyAllShips();

	makeCapitol(randStar);
	randStar->factionTakeOwnership(this, true);

	// Add a random starter weapon
	int rnd = Random::randInt(1, 3);
	if (rnd == 1) {
		addWeapon(Spaceship::DesignerWeapon("LASER_GUN"));
	}
	else if (rnd == 2) {
		addWeapon(Spaceship::DesignerWeapon("MACHINE_GUN"));
	}
	else if (rnd == 3) {
		addWeapon(Spaceship::DesignerWeapon("ROCKET_LAUNCHER"));
	}

	for (int i = 0; i < 10; i++) {
		sf::Vector2f pos = sf::Vector2f(Random::randFloat(-10000.0f, 10000.0f), Random::randFloat(-10000.0f, 10000.0f));
		addSpaceship(m_capitol->createSpaceship(std::make_unique<Spaceship>("FRIGATE_1", pos, m_capitol, m_id, m_color)));
		m_ships.back()->addWeapon(Weapon(m_weapons.back().type));
	}

	for (int i = 0; i < 3; i++) {
		sf::Vector2f pos = m_capitol->getRandomLocalPos(-10000.0f, 10000.0f);
		addSpaceship(m_capitol->createSpaceship(std::make_unique<Spaceship>("CONSTRUCTION_SHIP", pos, m_capitol, m_id, m_color)));
		m_ships.back()->addWeapon(Weapon("CONSTRUCTION_GUN"));
	}

	addSpaceship(m_capitol->createSpaceship(std::make_unique<Spaceship>("DESTROYER_1", Random::randVec(-10000, 10000), m_capitol, m_id, m_color)));
	m_ships.back()->addWeapon(Weapon("GAUSS_CANNON"));

	m_capitol->createBuilding(std::make_unique<Building>("OUTPOST", m_capitol, m_capitol->getRandomLocalPos(-10000, 10000), this));
	m_capitol->createBuilding(std::make_unique<Building>("SHIP_FACTORY", m_capitol, m_capitol->getRandomLocalPos(-10000.0f, 10000.0f), this));
	m_capitol->createBuilding(std::make_unique<Building>("SPACE_HABITAT", m_capitol, m_capitol->getRandomLocalPos(-10000.0f, 10000.0f), this));

	addResource("COMMON_ORE", 100.0f);
	
	addChassis(Spaceship::DesignerChassis("FRIGATE"));
	addChassis(Spaceship::DesignerChassis("DESTROYER"));
	addChassis(Spaceship::DesignerChassis("CONSTRUCTOR"));

	Spaceship::DesignerShip starterFrig;
	starterFrig.chassis = Spaceship::DesignerChassis("FRIGATE");
	starterFrig.weapons.push_back(Spaceship::DesignerWeapon(m_weapons.back().type));
	starterFrig.name = starterFrig.generateName();
	addOrReplaceDesignerShip(starterFrig);

	Spaceship::DesignerShip constructor;
	constructor.name = "Constructor";
	constructor.chassis = Spaceship::DesignerChassis("CONSTRUCTOR");
	addOrReplaceDesignerShip(constructor);

	const toml::table& table = TOMLCache::getTable("data/objects/colonybuildings.toml");
	for (auto& val : table) {
		addColonyBuilding(val.first);
	}

	if (m_aiEnabled) m_ai.onSpawn(this);
}

void Faction::addOwnedSystem(Star* star) {
	m_ownedSystems.push_back(star);
	m_ownedSystemIDs.push_back(star->getID());
	if (m_aiEnabled) m_ai.onStarTakeover(this, star);
}

void Faction::makeCapitol(Star* star) {
	m_capitol = star;
	m_capitolID = star->getID();
}

void Faction::update() {
	if (m_capitol->getAllegiance() != m_id) {
		if (m_ownedSystems.size() > 0) {
			makeCapitol(m_ownedSystems[0]);
		}
		else {
			if (m_ships.size() == 0) {
				m_dead = true;
				return;
			}
		}
	}
	
	for (int i = 0; i < m_ships.size(); i++) {
		m_ships[i]->captureCurrentStar(this);
	}
		
	if (m_aiEnabled) m_ai.controlFaction(this);
	
	// Delete unowned systems from list
	for (int i = 0; i < m_ownedSystems.size(); i++) {
		if (m_ownedSystems[i]->getAllegiance() != m_id) {
			m_ownedSystems.erase(m_ownedSystems.begin() + i);
			m_ownedSystemIDs.erase(m_ownedSystemIDs.begin() + i);
			i--;
		}
	}

	// Delete dead ships
	for (int i = 0; i < m_ships.size(); i++) {
		if (m_ships[i]->isDead()) {
			m_ships.erase(m_ships.begin() + i);
			m_shipIDs.erase(m_shipIDs.begin() + i);
			i--;
		}
	}
}

void Faction::controlByPlayer(Player& player) {
	player.setFaction(m_id, m_color);
	player.enableFogOfWar();
	m_aiEnabled = false;
}

std::vector<Spaceship*> Faction::getConstructionShips(bool onlyIdleShips) {
	std::vector<Spaceship*> constructionShips;
	for (Spaceship* ship : m_ships) {
		if (ship->getConstructionSpeed() > 0.0f) {
			if (onlyIdleShips && ship->numOrders() > 0) {
				continue;
			}
			constructionShips.push_back(ship);
		}
	}
	return constructionShips;
}

void Faction::orderConstructionShipsBuild(Building* building, bool onlyIdleShips, bool onlyOne) {
	if (building->getAllegiance() == getID() && !building->isBuilt()) {
		for (Spaceship* ship : getConstructionShips()) {
			if (onlyIdleShips && ship->numOrders() > 0) {
				continue;
			}
			if (building->getCurrentStar() != ship->getCurrentStar()) {
				ship->addOrder(TravelOrder(building->getCurrentStar()));
			}
			ship->addOrder(InteractWithBuildingOrder(building));
			if (onlyOne) break;
		}
	}
}

int Faction::numUnbuiltBuildings(Star* star) {
	int c = 0;
	for (auto& building : star->getBuildings()) {
		if (building->getAllegiance() == m_id && !building->isBuilt()) {
			c++;
		}
	}
	return c;
}

int Faction::numIdleConstructionShips() {
	return getConstructionShips(true).size();
}

void Faction::addResource(const std::string& type, float num) {
	m_resources[type] += num;
}

bool Faction::canSubtractResource(const std::string& type, float num) {
	if (m_resources.count(type) == 0) return false;
	if (m_resources[type] - num < 0) return false;
	return true;
}

void Faction::subtractResource(const std::string& type, float num) {
	if (canSubtractResource(type, num)) m_resources[type] -= num;
}

std::vector<Building*> Faction::getAllOwnedBuildings() {
	std::vector<Building*> buildings;
	for (Star* star : m_ownedSystems) {
		for (auto& building : star->getBuildings()) {
			if (building->getAllegiance() == m_id) {
				buildings.push_back(building.get());
			}
		}
	}
	return buildings;
}

std::vector<Building*> Faction::getAllOwnedBuildingsOfName(const std::string& name) {
	std::vector<Building*> buildings;
	for (Building* building : getAllOwnedBuildings()) {
		if (building->getName() == name) {
			buildings.push_back(building);
		}
	}
	return buildings;
}

std::vector<Spaceship*> Faction::getAllCombatShips() {
	std::vector<Spaceship*> ships;
	for (Spaceship* ship : m_ships) {
		if (!ship->isCivilian() && ship->getWeapons().size() > 0 && ship->getConstructionSpeed() == 0.0f) {
			ships.push_back(ship);
		}
	}
	return ships;
}

Spaceship::DesignerChassis Faction::getChassisByName(const std::string& name) {
	for (auto& chassis : m_chassis) {
		if (chassis.name == name) {
			return chassis;
		}
	}
	DEBUG_PRINT("Failed to get chassis");
	return Spaceship::DesignerChassis();
}

Spaceship::DesignerWeapon Faction::getWeaponByName(const std::string& name) {
	for (auto& weapon : m_weapons) {
		if (weapon.name == name) {
			return weapon;
		}
	}
	DEBUG_PRINT("Failed to get weapon");
	return Spaceship::DesignerWeapon();
}

void Faction::addOrReplaceDesignerShip(const Spaceship::DesignerShip& ship) {
	for (Spaceship::DesignerShip& designerShip : m_designerShips) {
		if (ship.name == designerShip.name) {
			designerShip = ship;
			return;
		}
	}

	m_designerShips.push_back(ship);
}

Spaceship::DesignerShip Faction::getShipDesignByName(const std::string& name) {
	for (auto& ship : m_designerShips) {
		if (ship.name == name) {
			return ship;
		}
	}
	DEBUG_PRINT("Failed to get ship design");
	return Spaceship::DesignerShip();
}

bool Faction::canSubtractResources(const std::unordered_map<std::string, float>& resources) {
	for (auto& resource : resources) {
		if (!canSubtractResource(resource.first, resource.second)) return false;
	}
	return true;
}

void Faction::subtractResources(const std::unordered_map<std::string, float>& resources) {
	for (auto& resource : resources) {
		subtractResource(resource.first, resource.second);
	}
}

void Faction::addAnnouncementEvent(const std::string& text) {
	if (!m_aiEnabled) {
		m_announcementEvents.push_back(text);
	}
}

std::vector<Spaceship::DesignerWeapon> Faction::getWeaponsBelowOrEqualWeaponPoints(float wp) {
	std::vector<Spaceship::DesignerWeapon> weapons;
	for (auto& weapon : m_weapons) {
		if (weapon.weaponPoints <= wp) {
			weapons.push_back(weapon);
		}
	}
	return weapons;
}

bool Faction::hasWeapon(const std::string& type) {
	for (auto& weapon : m_weapons) {
		if (weapon.type == type) {
			return true;
		}
	}
	return false;
}

void Faction::reinitAfterLoad(Constellation* constellation) {
	m_capitol = constellation->getStarByID(m_capitolID);
	
	for (uint32_t id : m_ownedSystemIDs) {
		m_ownedSystems.push_back(constellation->getStarByID(id));
	}

	for (uint32_t id : m_shipIDs) {
		m_ships.push_back(constellation->getShipByID(id));
	}
}

void Faction::addColonyBuilding(const std::string& type) {
	if (std::find(m_availableColonyBuildings.begin(), m_availableColonyBuildings.end(), type) == m_availableColonyBuildings.end()) {
		m_availableColonyBuildings.push_back(type);
	}
}

std::vector<ColonyBuilding> Faction::getColonyBuildings() {
	std::vector<ColonyBuilding> buildings;
	for (std::string& type : m_availableColonyBuildings) {
		buildings.push_back(ColonyBuilding(type));
	}
	return buildings;
}

Planet* Faction::getMostHabitablePlanet() {
	Planet* mostHabitable = nullptr;
	for (Star* star : m_ownedSystems) {
		for (Planet& planet : star->getPlanets()) {
			if (planet.getColony().getAllegiance() == m_id) {
				if (mostHabitable != nullptr) {
					if (planet.getHabitability() > mostHabitable->getHabitability()) {
						mostHabitable = &planet;
					}
				}
				else {
					mostHabitable = &planet;
				}
			}
		}
	}
	return mostHabitable;
}