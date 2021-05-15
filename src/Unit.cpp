#include "gamepch.h"
#include "Unit.h"
#include "Star.h"

std::vector<Spaceship*> Unit::findEnemyShips() {
	std::vector<Spaceship*>& allShips = m_currentStar->getSpaceships();
	std::vector<Spaceship*> enemies;
	for (Spaceship* s : allShips) {
		if (s->getAllegiance() != m_allegiance) {
			enemies.push_back(s);
		}
	}
	return enemies;
}

float Unit::getLongestWeaponRange() {
	float highest = 0.0f;
	
	for (Weapon& weapon : m_weapons) {
		if (weapon.getRange() > highest) {
			highest = weapon.getRange();
		}
	}

	return highest;
}

void Unit::updateWeapons() {
	for (Weapon& weapon : m_weapons) {
		weapon.update();
	}
}

std::vector<Unit*> Unit::findEnemyUnits() {
	std::vector<Unit*> units;
	for (Spaceship* ship : m_currentStar->getSpaceships()) {
		if (ship->getAllegiance() != m_allegiance) {
			units.push_back(ship);
		}
	}
	for (auto& building : m_currentStar->getBuildings()) {
		if (building->getAllegiance() != m_allegiance) {
			units.push_back(building.get());
		}
	}
	return units;
}

void Unit::fireAllWeaponsAt(Unit* target) {
	for (Weapon& weapon : m_weapons) {
		weapon.fireAt(this, target->getPos(), m_currentStar);
	}
}