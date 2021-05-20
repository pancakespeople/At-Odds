#include "gamepch.h"

#include "Faction.h"
#include "Debug.h"
#include "Constellation.h"
#include "Random.h"
#include "Player.h"
#include "Building.h"

Faction::Faction(Constellation* constellation) {
	m_constellation = constellation;
	m_color = sf::Color(rand() % 255, rand() % 255, rand() % 255);
	m_ticksUntilNextAIAction = 100;

	static int numFactions = 0;

	m_id = numFactions;
	numFactions++;

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
		m_ships.push_back(m_capitol->createSpaceship(std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::FRIGATE_1, pos, m_capitol, m_id, m_color)));
	}

	for (int i = 0; i < 3; i++) {
		sf::Vector2f pos = m_capitol->getRandomLocalPos(-10000.0f, 10000.0f);
		m_ships.push_back(m_capitol->createSpaceship(std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::CONSTRUCTION_SHIP, pos, m_capitol, m_id, m_color)));
	}
	
	m_ships.push_back(m_capitol->createSpaceship(std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::DESTROYER_1, Random::randVec(-10000, 10000), m_capitol, m_id, m_color)));
	m_capitol->createBuilding(std::make_unique<Building>(Building::BUILDING_TYPE::OUTPOST, m_capitol, m_capitol->getRandomLocalPos(-10000, 10000), m_id, m_color));

	if (m_aiEnabled) m_ai.onSpawn(this);
}

void Faction::addOwnedSystem(Star* star) {
	m_ownedSystems.push_back(star);
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
			return;
		}
	}
	
	for (int i = 0; i < m_ships.size(); i++) {
		m_ships[i]->captureCurrentStar(this);
	}
		
	if (m_aiEnabled) m_ai.controlFaction(this);
	
	if (m_freeShipTimer <= 0) {
		sf::Vector2f pos = sf::Vector2f(Random::randFloat(-10000.0f, 10000.0f), Random::randFloat(-10000.0f, 10000.0f));
		
		if (m_numFreeShips % 10 == 0 && m_numFreeShips != 0) {
			m_ships.push_back(m_capitol->createSpaceship(std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::DESTROYER_1, pos, m_capitol, m_id, m_color)));
			m_ships.push_back(m_capitol->createSpaceship(std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::CONSTRUCTION_SHIP, pos, m_capitol, m_id, m_color)));
		}
		else {
			m_ships.push_back(m_capitol->createSpaceship(std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::FRIGATE_1, pos, m_capitol, m_id, m_color)));
		}

		m_numFreeShips++;
		m_freeShipTimer = 2000;
	}
	else {
		m_freeShipTimer -= 1 + (m_ownedSystems.size() * 0.25);
	}

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