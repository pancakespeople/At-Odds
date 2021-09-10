#include "gamepch.h"
#include "Unit.h"
#include "Star.h"
#include "Faction.h"

std::vector<Spaceship*> Unit::findEnemyShips() {
	std::vector<std::unique_ptr<Spaceship>>& allShips = m_currentStar->getSpaceships();
	std::vector<Spaceship*> enemies;
	for (auto& s : allShips) {
		if (s->getAllegiance() != m_allegiance) {
			enemies.push_back(s.get());
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
	for (auto& ship : m_currentStar->getSpaceships()) {
		if (ship->getAllegiance() != m_allegiance) {
			units.push_back(ship.get());
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

void Unit::updateMods(Star* currentStar, Faction* faction) {
	for (auto& mod : m_mods) {
		mod->update(this, currentStar, faction);
	}
}

void Unit::enableAllMods() {
	for (auto& mod : m_mods) {
		mod->enable();
	}
}

void Unit::onDeath(Star* currentStar) {
	for (auto& mod : m_mods) {
		mod->onUnitDeath(currentStar);
	}
}

void Unit::openModGUI(tgui::ChildWindow::Ptr window, Faction* faction) {
	for (auto& mod : m_mods) {
		mod->openGUI(window, faction);
	}
}

void Unit::addWeapon(Weapon weapon) {
	if (m_weapons.size() > 0) {
		// Decrease accuracy for subsequent weapons
		float fallOff = 1.0f - m_weapons.size() * 0.05;
		weapon.setAccuracy(weapon.getAccuracy() * fallOff);
	}
	m_weapons.push_back(weapon);
}

float Unit::maxWeaponDamage() const {
	float max = 0.0f;
	for (const Weapon& weapon : m_weapons) {
		if (weapon.getDamage() > max) max = weapon.getDamage();
	}
	return max;
}