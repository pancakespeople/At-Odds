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

BOOST_CLASS_EXPORT_GUID(FlyToOrder, "FlyToOrder")
BOOST_CLASS_EXPORT_GUID(JumpOrder, "JumpOrder")
BOOST_CLASS_EXPORT_GUID(AttackOrder, "AttackOrder")
BOOST_CLASS_EXPORT_GUID(TravelOrder, "TravelOrder")
BOOST_CLASS_EXPORT_GUID(InteractWithBuildingOrder, "InteractWithBuildingOrder")

bool FlyToOrder::execute(Spaceship* ship, Star* currentStar) {
	return ship->flyTo(m_pos);
}

void FlyToOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) {
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
		std::vector<Spaceship*> enemies = ship->findEnemyShips();

		if (enemies.size() > 0) {
			ship->attackRandomEnemy(enemies, true);
			return false;
		}

	}
	if (ship->jump(m_jumpPoint) == Spaceship::JumpState::DONE) {
		return true;
	}
	else {
		return false;
	}
}

void JumpOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) {
	if (m_jumpPoint != nullptr)
		emitter.drawLine(window, shipPos, m_jumpPoint->getPos(), sf::Color::Yellow);
}

AttackOrder::AttackOrder(Unit* target) {
	m_targetID = target->getID();
	m_lastEnemyHealth = target->getHealth();
	m_frustration = 0.0f;
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
	
	if (m_target->getVelocity() == sf::Vector2f(0.0f, 0.0f)) {
		ship->orbit(m_target->getPos());
	}
	else {
		ship->flyTo(m_target->getPos());
	}

	float dist = Math::distance(ship->getPos(), m_target->getPos());
	
	std::vector<Weapon>& weapons = ship->getWeapons();

	for (int i = 0; i < weapons.size(); i++) {
		if (dist < weapons[i].getRange()) {
			ship->smartFireAt(m_target, i);
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

void AttackOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) {
	if (m_target != nullptr) {
		emitter.drawLine(window, shipPos, m_target->getPos(), sf::Color::Red);
	}
}

TravelOrder::TravelOrder(Star* star) {
	m_endStar = star;
}

bool TravelOrder::execute(Spaceship* ship, Star* currentStar) {
	if (!m_pathFound) {
		m_path = Pathfinder::findPath(ship->getCurrentStar(), m_endStar);
		m_pathFound = true;
		return false;
	}
	else if (m_path.size() == 0) {
		return true;
	}
	else if (ship->getCurrentStar() == m_path.front()) {
		m_path.pop_front();
		return false;
	}
	else {
		for (JumpPoint& j : ship->getCurrentStar()->getJumpPoints()) {
			if (j.getConnectedOtherStar() == m_path.front()) {
				ship->addOrderFront(JumpOrder(&j, true));
				return false;
			}
		}
	}
	return false;
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

void InteractWithBuildingOrder::draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) {
	if (m_building != nullptr)
		emitter.drawLine(window, shipPos, m_building->getPos(), sf::Color(100, 100, 255));
}