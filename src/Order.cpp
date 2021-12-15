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

BOOST_CLASS_EXPORT_GUID(FlyToOrder, "FlyToOrder")
BOOST_CLASS_EXPORT_GUID(JumpOrder, "JumpOrder")
BOOST_CLASS_EXPORT_GUID(AttackOrder, "AttackOrder")
BOOST_CLASS_EXPORT_GUID(TravelOrder, "TravelOrder")
BOOST_CLASS_EXPORT_GUID(InteractWithBuildingOrder, "InteractWithBuildingOrder")
BOOST_CLASS_EXPORT_GUID(InteractWithPlanetOrder, "InteractWithPlanetOrder")
BOOST_CLASS_EXPORT_GUID(DieOrder, "DieOrder")
BOOST_CLASS_EXPORT_GUID(InteractWithUnitOrder, "InteractWithUnitOrder")
BOOST_CLASS_EXPORT_GUID(EstablishPirateBaseOrder, "EstablishPirateBaseOrder")

bool FlyToOrder::execute(Spaceship* ship, Star* currentStar) {
	return ship->flyTo(m_pos);
}

void FlyToOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos, Star* currentStar) {
	emitter.drawLine(window, shipPos, m_pos, sf::Color::Green);
}

JumpOrder::JumpOrder(JumpPoint* point, bool attackEnemies) {
	m_jumpPointID = point->getID();
	m_attackEnemies = attackEnemies;
}

bool JumpOrder::execute(Spaceship* ship, Star* currentStar) {
	if (m_jumpPoint == nullptr) {
		m_jumpPoint = currentStar->getJumpPointByID(m_jumpPointID);
		if (m_jumpPoint == nullptr) {
			return true;
		}
	}
	
	if (m_attackEnemies) {
		bool attacking = ship->attack(currentStar, true);
		if (attacking) return false;
	}
	if (ship->jump(m_jumpPoint) == Spaceship::JumpState::DONE) {
		return true;
	}
	else {
		return false;
	}
}

void JumpOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos, Star* currentStar) {
	m_jumpPoint = currentStar->getJumpPointByID(m_jumpPointID);
	if (m_jumpPoint != nullptr)
		emitter.drawLine(window, shipPos, m_jumpPoint->getPos(), sf::Color::Yellow);
}

AttackOrder::AttackOrder(Unit* target, bool aggressive) {
	m_targetID = target->getID();
	m_lastEnemyHealth = target->getHealth();
	m_frustration = 0.0f;
	m_aggressive = aggressive;
}

bool AttackOrder::execute(Spaceship* ship, Star* currentStar) {
	if (m_target == nullptr) {
		m_target = currentStar->getUnitByID(m_targetID);
		if (m_target == nullptr)
			return true;
	}
	
	if (m_target->getCurrentStar() != ship->getCurrentStar() || m_target->getAllegiance() == ship->getAllegiance()) {
		return true;
	}
	
	if (Math::magnitude(m_target->getVelocity()) < 1.0f || m_aggressive) {
		ship->orbit(m_target->getPos());
	}
	else {
		ship->flyTo(m_target->getPos());
	}

	float dist = Math::distance(ship->getPos(), m_target->getPos());
	
	std::vector<Weapon>& weapons = ship->getWeapons();

	for (int i = 0; i < weapons.size(); i++) {
		if (dist < weapons[i].getRange()) {
			
			if (!weapons[i].isInstaHit()) {
				ship->smartFireAt(m_target, i);
			}
			else {
				weapons[i].instaHitFireAt(ship->getPos(), m_target, currentStar);
			}

			if (ship->getHealth() < m_lastEnemyHealth) {
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

void AttackOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos, Star* currentStar) {
	m_target = currentStar->getUnitByID(m_targetID);
	if (m_target != nullptr) {
		emitter.drawLine(window, shipPos, m_target->getPos(), sf::Color::Red);
	}
}

std::pair<bool, sf::Vector2f> AttackOrder::getDestinationPos(Star* currentStar) {
	m_target = currentStar->getUnitByID(m_targetID);
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

bool TravelOrder::execute(Spaceship* ship, Star* currentStar) {
	if (!m_pathFound) {
		m_path = Pathfinder::findPath(ship->getCurrentStar(), m_endStar);

		for (Star* star : m_path) {
			m_pathIDs.push_back(star->getID());
		}

		m_pathFound = true;
		return false;
	}
	else if (m_path.size() == 0) {
		return true;
	}
	else if (ship->getCurrentStar() == m_path.front()) {
		m_path.pop_front();
		m_pathIDs.pop_front();
		return false;
	}
	else {
		for (JumpPoint& j : ship->getCurrentStar()->getJumpPoints()) {
			if (j.getConnectedOtherStar() == m_path.front()) {
				ship->addOrderFront(JumpOrder(&j, !ship->isCivilian()));
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

bool InteractWithBuildingOrder::execute(Spaceship* ship, Star* currentStar) {
	if (m_building == nullptr) {
		m_building = currentStar->getBuildingByID(m_buildingID);
		if (m_building == nullptr) {
			return true;
		}
	}
	
	if (m_building->getCurrentStar() != ship->getCurrentStar() || m_building->isDead()) {
		return true;
	}
	if (ship->getConstructionSpeed() > 0.0) {
		// Construction ship
		float insideRadius = m_building->getCollider().getRadius() * 4.0f;
		if (Math::distance(ship->getPos(), m_building->getPos()) < insideRadius) {
			if (!m_building->isBuilt()) {
				m_building->construct(ship);
				ship->fireAllWeaponsAt(m_building);
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
				ship->flyTo(m_building->getPos());
				return false;
			}
		}
	}
	else {
		return ship->flyTo(m_building->getPos());
	}
}

void InteractWithBuildingOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos, Star* currentStar) {
	m_building = currentStar->getBuildingByID(m_buildingID);
	if (m_building != nullptr)
		emitter.drawLine(window, shipPos, m_building->getPos(), sf::Color(100, 100, 255));
}

std::pair<bool, sf::Vector2f> InteractWithBuildingOrder::getDestinationPos(Star* currentStar) {
	m_building = currentStar->getBuildingByID(m_buildingID);
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

bool InteractWithPlanetOrder::execute(Spaceship* ship, Star* currentStar) {
	if (m_planet == nullptr) {
		m_planet = currentStar->getPlanetByID(m_planetID);
		if (m_planet == nullptr) {
			return true;
		}
	}

	ship->orbit(m_planet->getPos());

	if (Math::distance(ship->getPos(), m_planet->getPos()) < (m_planet->getRadius() + ship->getCollider().getRadius())) {
		
		// Mods interact if civilian or friendly planet
		if (ship->isCivilian() || ship->getAllegiance() == m_planet->getColony().getAllegiance()) {
			for (auto& mod : ship->getMods()) {
				mod->interactWithPlanet(ship, m_planet, currentStar);
			}
			return true;
		}
	}

	if (!ship->isCivilian() && ship->getAllegiance() != m_planet->getColony().getAllegiance()) {
		// Orbital bombardment

		if (m_planet->getColony().getPopulation() == 0) return true;

		auto& weapons = ship->getWeapons();
		for (Weapon& weapon : weapons) {
			float dist = Math::distance(ship->getPos(), m_planet->getPos());
			if ((weapon.getProjectile().canOrbitallyBombard() || weapon.getProjectile().canInvadePlanets()) && !weapon.isOnCooldown() && dist < weapon.getRange()) {
				Projectile newProj = weapon.getProjectile();
				newProj.setPos(ship->getPos());
				newProj.setAllegiance(ship->getAllegiance());

				m_planet->addBombardProjectile(newProj);
				weapon.triggerCooldown();
				weapon.playFireSound(ship->getPos(), currentStar);
			}
		}
	}

	// No interaction for combat ships on friendly planets yet
	if (m_planet->getColony().getAllegiance() == ship->getAllegiance() && !ship->isCivilian()) return true;

	return false;
}

void InteractWithPlanetOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos, Star* currentStar) {
	m_planet = currentStar->getPlanetByID(m_planetID);
	if (m_planet != nullptr)
		emitter.drawLine(window, shipPos, m_planet->getPos(), sf::Color(100, 100, 255));
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

bool DieOrder::execute(Spaceship* ship, Star* currentStar) {
	ship->kill();
	if (m_dieSilently) ship->setSilentDeath(true);
	return true;
}

InteractWithUnitOrder::InteractWithUnitOrder(Unit* unit) {
	m_unitID = unit->getID();
}

bool InteractWithUnitOrder::execute(Spaceship* ship, Star* currentStar) {
	if (m_unit == nullptr) {
		m_unit = currentStar->getUnitByID(m_unitID);
		if (m_unit == nullptr) {
			return true;
		}
	}

	if (m_unit->getCurrentStar() != ship->getCurrentStar() || m_unit->isDead()) {
		return true;
	}

	if (ship->flyTo(m_unit->getPos())) {
		for (auto& mod : m_unit->getMods()) {
			mod->onShipInteract(ship);
		}
		return true;
	}

	return false;
}

void InteractWithUnitOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos, Star* currentStar) {
	m_unit = currentStar->getBuildingByID(m_unitID);
	if (m_unit != nullptr)
		emitter.drawLine(window, shipPos, m_unit->getPos(), sf::Color(100, 100, 255));
}

std::pair<bool, sf::Vector2f> InteractWithUnitOrder::getDestinationPos(Star* currentStar) {
	m_unit = currentStar->getUnitByID(m_unitID);
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

bool EstablishPirateBaseOrder::execute(Spaceship* ship, Star* currentStar) {
	if (ship->flyTo(m_pos)) {
		Building* building = currentStar->createBuilding("PIRATE_BASE", m_pos, nullptr, true);
		PirateBaseMod* pirateBase = building->getMod<PirateBaseMod>();
		pirateBase->setTheftAllegiance(m_theftAllegiance);
		
		for (const DesignerShip& design : m_designs) {
			pirateBase->addDesign(design);
		}

		return true;
	}
	return false;
}