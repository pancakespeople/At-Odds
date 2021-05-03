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