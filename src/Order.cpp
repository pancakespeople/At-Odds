#include "gamepch.h"

#include "Order.h"
#include "Spaceship.h"
#include "Debug.h"
#include "Math.h"
#include "Pathfinder.h"
#include "Star.h"
#include "Building.h"
#include "SaveLoader.h"
#include "Hyperlane.h"
#include "Planet.h"
#include "Constellation.h"
#include "Designs.h"
#include "Renderer.h"
#include "Random.h"
#include "Asteroid.h"

BOOST_CLASS_EXPORT_GUID(FlyToOrder, "FlyToOrder")
BOOST_CLASS_EXPORT_GUID(JumpOrder, "JumpOrder")
BOOST_CLASS_EXPORT_GUID(AttackOrder, "AttackOrder")
BOOST_CLASS_EXPORT_GUID(TravelOrder, "TravelOrder")
BOOST_CLASS_EXPORT_GUID(InteractWithBuildingOrder, "InteractWithBuildingOrder")
BOOST_CLASS_EXPORT_GUID(InteractWithPlanetOrder, "InteractWithPlanetOrder")
BOOST_CLASS_EXPORT_GUID(DieOrder, "DieOrder")
BOOST_CLASS_EXPORT_GUID(InteractWithUnitOrder, "InteractWithUnitOrder")
BOOST_CLASS_EXPORT_GUID(EstablishPirateBaseOrder, "EstablishPirateBaseOrder")

bool FlyToOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	return ship.flyTo(m_pos);
}

void FlyToOrder::draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) {
	renderer.effects.drawLine(shipPos, m_pos, sf::Color::Green);
}

JumpOrder::JumpOrder(JumpPoint* point, bool attackEnemies) {
	m_jumpPointID = point->getID();
	m_attackEnemies = attackEnemies;
}

bool JumpOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	if (m_jumpPoint == nullptr) {
		m_jumpPoint = currentStar.getJumpPointByID(m_jumpPointID);
		if (m_jumpPoint == nullptr) {
			return true;
		}
	}
	
	if (m_attackEnemies) {
		bool attacking = ship.attack(&currentStar, alliances, true);
		if (attacking) return false;
	}
	if (ship.jump(m_jumpPoint) == Spaceship::JumpState::DONE) {
		return true;
	}
	else {
		return false;
	}
}

void JumpOrder::draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) {
	if (!executing) m_jumpPoint = currentStar->getJumpPointByID(m_jumpPointID);
	if (m_jumpPoint != nullptr)
		renderer.effects.drawLine(shipPos, m_jumpPoint->getPos(), sf::Color::Yellow);
}

AttackOrder::AttackOrder(Unit* target, bool aggressive) {
	m_targetID = target->getID();
	m_lastEnemyHealth = target->getHealth();
	m_frustration = 0.0f;
	m_aggressive = aggressive;
}

bool AttackOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	if (m_target == nullptr) {
		m_target = currentStar.getUnitByID(m_targetID);
		if (m_target == nullptr)
			return true;
	}
	
	if (m_target->getCurrentStar() != ship.getCurrentStar() || alliances.isAllied(m_target->getAllegiance(), ship.getAllegiance())) {
		return true;
	}
	
	if (Math::magnitude(m_target->getVelocity()) < 1.0f || m_aggressive) {
		ship.orbit(m_target->getPos());
	}
	else {
		ship.flyTo(m_target->getPos());
	}

	float dist = Math::distance(ship.getPos(), m_target->getPos());
	
	std::vector<Weapon>& weapons = ship.getWeapons();

	for (int i = 0; i < weapons.size(); i++) {
		if (dist < weapons[i].getRange()) {
			
			if (!weapons[i].isInstaHit()) {
				ship.smartFireAt(m_target, i);
			}
			else {
				weapons[i].instaHitFireAt(ship.getPos(), m_target->getPos(), m_target, &currentStar);
			}

			if (ship.getHealth() < m_lastEnemyHealth) {
				m_frustration = 0.0f;
			}
			else {
				m_frustration += 1.0f / weapons.size();
			}
		}
		else if (dist < weapons[i].getRange() * 5.0f) {
			m_frustration += 1.0f / weapons.size();
		}
	}
	
	m_lastEnemyHealth = m_target->getHealth();

	if (m_target->isDead() || m_frustration > 300) {
		return true;
	}
	else {
		return false;
	}
}

void AttackOrder::draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) {
	if (!executing) m_target = currentStar->getUnitByID(m_targetID); // This is here so queued orders can still track their targets
	if (m_target != nullptr) {
		renderer.effects.drawLine(shipPos, m_target->getPos(), sf::Color::Red);
	}
}

std::pair<bool, sf::Vector2f> AttackOrder::getDestinationPos(Star* currentStar) {
	if (!executing) m_target = currentStar->getUnitByID(m_targetID);
	if (m_target != nullptr) {
		return std::pair<bool, sf::Vector2f>(true, m_target->getPos());
	}
	else {
		return std::pair<bool, sf::Vector2f>(false, sf::Vector2f());
	}
}

TravelOrder::TravelOrder(Star* star) {
	m_endStar = star;
	m_endStarID = star->getID();
}

bool TravelOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	if (!m_pathFound) {
		m_path = Pathfinder::findPath(ship.getCurrentStar(), m_endStar);

		for (Star* star : m_path) {
			m_pathIDs.push_back(star->getID());
		}

		m_pathFound = true;
		return false;
	}
	else if (m_path.size() == 0) {
		return true;
	}
	else if (ship.getCurrentStar() == m_path.front()) {
		m_path.pop_front();
		m_pathIDs.pop_front();
		return false;
	}
	else {
		for (JumpPoint& j : ship.getCurrentStar()->getJumpPoints()) {
			if (j.getConnectedOtherStar() == m_path.front()) {
				ship.addOrderFront(JumpOrder(&j, !ship.isCivilian()));
				return false;
			}
		}
	}
	return false;
}

void TravelOrder::reinitAfterLoad(Constellation* constellation) {
	m_endStar = constellation->getStarByID(m_endStarID);
	for (uint32_t id : m_pathIDs) {
		m_path.push_back(constellation->getStarByID(id));
	}
}

InteractWithBuildingOrder::InteractWithBuildingOrder(Building* building) {
	m_buildingID = building->getID();
}

bool InteractWithBuildingOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	if (m_building == nullptr) {
		m_building = currentStar.getBuildingByID(m_buildingID);
		if (m_building == nullptr) {
			return true;
		}
	}
	
	if (m_building->getCurrentStar() != ship.getCurrentStar() || m_building->isDead()) {
		return true;
	}
	if (ship.hasWeapon("CONSTRUCTION_GUN")) {
		// Construction ship
		float insideRadius = m_building->getCollider().getRadius() * 4.0f;
		if (Math::distance(ship.getPos(), m_building->getPos()) < insideRadius) {
			if (!m_building->isBuilt()) {
				m_building->construct(&ship);
				
				for (Weapon& weapon : ship.getWeapons()) {
					if (weapon.getType() == "CONSTRUCTION_GUN") {
						weapon.instaHitFireAt(ship.getPos(), m_building->getPos() + Random::randPointInCircle(m_building->getCollider().getRadius()), nullptr, &currentStar);
					}
				}

				return false;
			}
			else {
				m_building->enableAllMods();
				return true;
			}
		}
		else {
			if (m_building->isBuilt()) {
				return true;
			}
			else {
				ship.flyTo(m_building->getPos());
				return false;
			}
		}
	}
	else {
		return ship.flyTo(m_building->getPos());
	}
}

void InteractWithBuildingOrder::draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) {
	if (!executing) m_building = currentStar->getBuildingByID(m_buildingID);
	if (m_building != nullptr)
		renderer.effects.drawLine(shipPos, m_building->getPos(), sf::Color(100, 100, 255));
}

std::pair<bool, sf::Vector2f> InteractWithBuildingOrder::getDestinationPos(Star* currentStar) {
	if (!executing) m_building = currentStar->getBuildingByID(m_buildingID);
	if (m_building != nullptr) {
		return std::pair<bool, sf::Vector2f>(true, m_building->getPos());
	}
	else {
		return std::pair<bool, sf::Vector2f>(false, sf::Vector2f());
	}
}

InteractWithPlanetOrder::InteractWithPlanetOrder(Planet* planet, Star* star) {
	m_planetID = planet->getID();
}

bool InteractWithPlanetOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	if (m_planet == nullptr) {
		m_planet = currentStar.getPlanetByID(m_planetID);
		if (m_planet == nullptr) {
			return true;
		}
	}

	ship.orbit(m_planet->getPos());

	if (Math::distance(ship.getPos(), m_planet->getPos()) < (m_planet->getRadius() + ship.getCollider().getRadius())) {
		
		// Mods interact if civilian or friendly planet
		if (ship.isCivilian() || alliances.isAllied(ship.getAllegiance(), m_planet->getColony().getAllegiance())) {
			for (auto& mod : ship.getMods()) {
				mod->interactWithPlanet(&ship, m_planet, &currentStar);
			}
			return true;
		}
	}

	bool allied = alliances.isAllied(ship.getAllegiance(), m_planet->getColony().getAllegiance());

	if (!ship.isCivilian() && !allied) {
		// Orbital bombardment

		if (m_planet->getColony().getPopulation() == 0) return true;

		auto& weapons = ship.getWeapons();
		for (Weapon& weapon : weapons) {
			float dist = Math::distance(ship.getPos(), m_planet->getPos());
			if ((weapon.getProjectile().canOrbitallyBombard() || weapon.getProjectile().canInvadePlanets()) && !weapon.isOnCooldown() && dist < weapon.getRange()) {
				Projectile newProj = weapon.getProjectile();
				newProj.setPos(ship.getPos());
				newProj.setAllegiance(ship.getAllegiance());

				m_planet->addBombardProjectile(newProj);
				weapon.triggerCooldown();
				weapon.playFireSound(ship.getPos(), &currentStar);
			}
		}
	}

	// No interaction for combat ships on friendly planets yet
	if (allied && !ship.isCivilian()) return true;

	return false;
}

void InteractWithPlanetOrder::draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) {
	if (!executing) m_planet = currentStar->getPlanetByID(m_planetID);
	if (m_planet != nullptr)
		renderer.effects.drawLine(shipPos, m_planet->getPos(), sf::Color(100, 100, 255));
}

std::pair<bool, sf::Vector2f> InteractWithPlanetOrder::getDestinationPos(Star* currentStar) {
	m_planet = currentStar->getPlanetByID(m_planetID);
	if (m_planet != nullptr) {
		return std::pair<bool, sf::Vector2f>(true, m_planet->getPos());
	}
	else {
		return std::pair<bool, sf::Vector2f>(false, sf::Vector2f());
	}
}

DieOrder::DieOrder(bool silently) {
	m_dieSilently = silently;
}

bool DieOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	ship.kill();
	if (m_dieSilently) ship.setSilentDeath(true);
	return true;
}

InteractWithUnitOrder::InteractWithUnitOrder(Unit* unit) {
	m_unitID = unit->getID();
}

bool InteractWithUnitOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	if (m_unit == nullptr) {
		m_unit = currentStar.getUnitByID(m_unitID);
		if (m_unit == nullptr) {
			return true;
		}
	}

	if (m_unit->getCurrentStar() != ship.getCurrentStar() || m_unit->isDead()) {
		return true;
	}

	if (ship.flyTo(m_unit->getPos())) {
		for (auto& mod : m_unit->getMods()) {
			mod->onShipInteract(&ship);
		}
		return true;
	}

	return false;
}

void InteractWithUnitOrder::draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) {
	if (!executing) m_unit = currentStar->getBuildingByID(m_unitID);
	if (m_unit != nullptr)
		renderer.effects.drawLine(shipPos, m_unit->getPos(), sf::Color(100, 100, 255));
}

std::pair<bool, sf::Vector2f> InteractWithUnitOrder::getDestinationPos(Star* currentStar) {
	if (!executing) m_unit = currentStar->getUnitByID(m_unitID);
	if (m_unit != nullptr) {
		return std::pair<bool, sf::Vector2f>(true, m_unit->getPos());
	}
	else {
		return std::pair<bool, sf::Vector2f>(false, sf::Vector2f());
	}
}

EstablishPirateBaseOrder::EstablishPirateBaseOrder(sf::Vector2f pos, int theftAllegiance, const std::deque<DesignerShip>& designs) {
	m_pos = pos;
	m_theftAllegiance = theftAllegiance;
	m_designs = designs;
}

bool EstablishPirateBaseOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	if (currentStar.getBuildingsOfType("PIRATE_BASE").size() > 1) {
		// Pick a different star if this one already has 2 bases

		Star* rndNeighbor = currentStar.getConnectedStars()[Random::randInt(0, currentStar.getConnectedStars().size() - 1)];
		ship.addOrderFront<TravelOrder>(TravelOrder(rndNeighbor));
		
		return false;
	}

	if (ship.flyTo(m_pos)) {
		Building* building = currentStar.createBuilding("PIRATE_BASE", m_pos, nullptr, true);
		PirateBaseMod* pirateBase = building->getMod<PirateBaseMod>();
		pirateBase->setTheftAllegiance(m_theftAllegiance);
		
		for (const DesignerShip& design : m_designs) {
			pirateBase->addDesign(design);
		}

		return true;
	}
	return false;
}

bool MineAsteroidOrder::execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances) {
	if (m_asteroid == nullptr) {
		m_asteroid = currentStar.getAsteroidByID(m_asteroidID);
		if (m_asteroid == nullptr) {
			return true;
		}
	}

	if (ship.hasWeapon("MINING_LASER")) {
		Weapon* miningLaser = ship.getWeapon("MINING_LASER");

		if (Math::distance(ship.getPos(), m_asteroid->getPos()) - m_asteroid->getRadius() < miningLaser->getRange()) {
			miningLaser->instaHitFireAt(ship.getPos(), m_asteroid->getPos() + Random::randPointInCircle(m_asteroid->getRadius()), nullptr, &currentStar);
		}

		ship.orbit(m_asteroid->getPos());
	}
	else {
		return true;
	}

	return false;
}

MineAsteroidOrder::MineAsteroidOrder(Asteroid* asteroid) {
	m_asteroid = asteroid;
	m_asteroidID = asteroid->getID();
}

void MineAsteroidOrder::draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) {
	if (!executing) m_asteroid = currentStar->getAsteroidByID(m_asteroidID);
	if (m_asteroid != nullptr) {
		renderer.effects.drawLine(shipPos, m_asteroid->getPos(), sf::Color(255, 215, 0));
	}
}

std::pair<bool, sf::Vector2f> MineAsteroidOrder::getDestinationPos(Star* currentStar) {
	if (!executing) m_asteroid = currentStar->getAsteroidByID(m_asteroidID);
	if (m_asteroid != nullptr) {
		return std::pair<bool, sf::Vector2f>(true, m_asteroid->getPos());
	}
	else {
		return std::pair<bool, sf::Vector2f>(false, sf::Vector2f());
	}
}