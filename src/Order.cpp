#include "gamepch.h"

#include "Order.h"
#include "Spaceship.h"
#include "Debug.h"
#include "Math.h"
#include "Pathfinder.h"
#include "Star.h"

bool FlyToOrder::execute(Spaceship* ship) {
	return ship->flyTo(m_pos);
}

bool JumpOrder::execute(Spaceship* ship) {
	if (m_attackEnemies) {
		std::vector<Spaceship*> enemies = ship->findEnemies();

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

AttackOrder::AttackOrder(Spaceship* target) {
	m_target = target;
	m_lastEnemyHealth = target->getHealth();
	m_frustration = 0.0f;
}

bool AttackOrder::execute(Spaceship* ship) {
	if (m_target->getCurrentStar() != ship->getCurrentStar() || m_target->getAllegiance() == ship->getAllegiance()) {
		return true;
	}
	
	ship->flyTo(m_target->getPos());
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

TravelOrder::TravelOrder(Star* star) {
	m_endStar = star;
}

bool TravelOrder::execute(Spaceship* ship) {
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