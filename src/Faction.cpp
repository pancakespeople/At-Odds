#include "gamepch.h"

#include "Faction.h"
#include "Debug.h"
#include "Constellation.h"
#include "Random.h"
#include "Player.h"
#include "Building.h"
#include "ShipParts.h"

Faction::Faction(Constellation* constellation, int id) {
	m_constellation = constellation;
	m_color = sf::Color(rand() % 255, rand() % 255, rand() % 255);

	m_id = id;

	if (m_aiEnabled) m_ai.onStart(this);
}

void Faction::spawnAtRandomStar() {
	Star* randStar = m_constellation->getStars()[Random::randInt(0, m_constellation->getStars().size() - 1)].get();

	int iterations = 0;
	while (randStar->getAllegiance() != -1 && iterations < 50) {
		randStar = m_constellation->getStars()[Random::randInt(0, m_constellation->getStars().size() - 1)].get();
		iterations++;
	}

	randStar->destroyAllShips();

	makeCapitol(randStar);
	randStar->factionTakeOwnership(this, true);

	for (int i = 0; i < 10; i++) {
		sf::Vector2f pos = sf::Vector2f(Random::randFloat(-10000.0f, 10000.0f), Random::randFloat(-10000.0f, 10000.0f));
		m_ships.push_back(m_capitol->createSpaceship(std::make_unique<Spaceship>("FRIGATE_1", pos, m_capitol, m_id, m_color)));
		if (Random::randBool()) {
			m_ships.back()->addWeapon(Weapon(Weapon::WEAPON_TYPE::LASER_GUN));
		}
		else {
			m_ships.back()->addWeapon(Weapon(Weapon::WEAPON_TYPE::MACHINE_GUN));
		}
	}

	for (int i = 0; i < 3; i++) {
		sf::Vector2f pos = m_capitol->getRandomLocalPos(-10000.0f, 10000.0f);
		m_ships.push_back(m_capitol->createSpaceship(std::make_unique<Spaceship>("CONSTRUCTION_SHIP", pos, m_capitol, m_id, m_color)));
		m_ships.back()->addWeapon(Weapon(Weapon::WEAPON_TYPE::CONSTRUCTION_GUN));
	}

	m_ships.push_back(m_capitol->createSpaceship(std::make_unique<Spaceship>("DESTROYER_1", Random::randVec(-10000, 10000), m_capitol, m_id, m_color)));
	m_ships.back()->addWeapon(Weapon(Weapon::WEAPON_TYPE::GAUSS_CANNON));

	m_capitol->createBuilding(std::make_unique<Building>(Building::BUILDING_TYPE::OUTPOST, m_capitol, m_capitol->getRandomLocalPos(-10000, 10000), m_id, m_color));
	m_capitol->createBuilding(std::make_unique<Building>(Building::BUILDING_TYPE::SHIP_FACTORY, m_capitol, m_capitol->getRandomLocalPos(-10000.0f, 10000.0f), m_id, m_color));
	m_capitol->createBuilding(std::make_unique<Building>(Building::BUILDING_TYPE::SPACE_HABITAT, m_capitol, m_capitol->getRandomLocalPos(-10000.0f, 10000.0f), m_id, m_color));

	addResource(PlanetResource::RESOURCE_TYPE::COMMON_ORE, 100.0f);
	
	addChassis(ShipParts::frigateChassis);
	addChassis(ShipParts::destroyerChassis);
	addChassis(ShipParts::constructorChassis);

	addWeapon(ShipParts::laserGunWeapon);
	addWeapon(ShipParts::machineGunWeapon);
	addWeapon(ShipParts::gaussCannonWeapon);

	// Add starter ship designs
	Spaceship::DesignerShip laserFrig;
	laserFrig.name = "Laser Frigate";
	laserFrig.chassis = ShipParts::frigateChassis;
	laserFrig.weapons.push_back(ShipParts::laserGunWeapon);
	addOrReplaceDesignerShip(laserFrig);

	Spaceship::DesignerShip mgFrig;
	mgFrig.name = "MG Frigate";
	mgFrig.chassis = ShipParts::frigateChassis;
	mgFrig.weapons.push_back(ShipParts::machineGunWeapon);
	addOrReplaceDesignerShip(mgFrig);

	Spaceship::DesignerShip dest;
	dest.name = "Destroyer";
	dest.chassis = ShipParts::destroyerChassis;
	dest.weapons.push_back(ShipParts::gaussCannonWeapon);
	addOrReplaceDesignerShip(dest);

	Spaceship::DesignerShip constructor;
	constructor.name = "Constructor";
	constructor.chassis = ShipParts::constructorChassis;
	addOrReplaceDesignerShip(constructor);

	if (m_aiEnabled) m_ai.onSpawn(this);
}

void Faction::addOwnedSystem(Star* star) {
	m_ownedSystems.push_back(star);
	if (m_aiEnabled) m_ai.onStarTakeover(this, star);
}

void Faction::makeCapitol(Star* star) {
	m_capitol = star;
}

void Faction::update() {
	if (m_capitol->getAllegiance() != m_id) {
		if (m_ownedSystems.size() > 0) {
			m_capitol = m_ownedSystems[0];
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
	m_ownedSystems.erase(std::remove_if(m_ownedSystems.begin(), m_ownedSystems.end(), [&](Star* s) {return s->getAllegiance() != m_id; }), m_ownedSystems.end());

	// Delete dead ships
	m_ships.erase(std::remove_if(m_ships.begin(), m_ships.end(), [](Spaceship* s) {return s->isDead(); }), m_ships.end());
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

void Faction::addResource(PlanetResource::RESOURCE_TYPE type, float num) {
	m_resources[type] += num;
}

bool Faction::canSubtractResource(PlanetResource::RESOURCE_TYPE type, float num) {
	if (m_resources.count(type) == 0) return false;
	if (m_resources[type] - num < 0) return false;
	return true;
}

void Faction::subtractResource(PlanetResource::RESOURCE_TYPE type, float num) {
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

std::vector<Building*> Faction::getAllOwnedBuildingsOfType(Building::BUILDING_TYPE type) {
	std::vector<Building*> buildings;
	for (Building* building : getAllOwnedBuildings()) {
		if (building->getType() == type) {
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

bool Faction::canSubtractResources(const std::unordered_map<PlanetResource::RESOURCE_TYPE, float>& resources) {
	for (auto& resource : resources) {
		if (!canSubtractResource(resource.first, resource.second)) return false;
	}
	return true;
}

void Faction::subtractResources(const std::unordered_map<PlanetResource::RESOURCE_TYPE, float>& resources) {
	for (auto& resource : resources) {
		subtractResource(resource.first, resource.second);
	}
}