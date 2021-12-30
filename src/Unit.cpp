#include "gamepch.h"
#include "Unit.h"
#include "Star.h"
#include "Faction.h"
#include "SmoothCircle.h"
#include "Constellation.h"
#include "Renderer.h"

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
		weapon.fireAt(getPos(), getAllegiance(), target->getPos(), m_currentStar);
	}
}

void Unit::updateMods(Star* currentStar, Faction* faction, Constellation* constellation) {
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

void Unit::drawSelectionCircle(Renderer& renderer) {
	sf::RectangleShape selectionIndicator;

	selectionIndicator.setOrigin(sf::Vector2f(m_collider.getRadius() * 2.0f, m_collider.getRadius() * 2.0f));
	selectionIndicator.setPosition(getPos());
	selectionIndicator.setFillColor(sf::Color::Yellow);
	selectionIndicator.setSize(sf::Vector2f(m_collider.getRadius() * 4.0f, m_collider.getRadius() * 4.0f));

	renderer.effects.drawSelection(renderer, selectionIndicator);
}

void Unit::drawHealthBar(Renderer& renderer) {
	sf::RectangleShape rect;

	float healthPercent = m_health / m_maxHealth;
	float sizeX = m_collider.getRadius() * 2.0f * healthPercent;
	float sizeY = 25.0f;

	rect.setSize(sf::Vector2f(sizeX, sizeY));
	rect.setOrigin(sizeX / 2.0f, 0.0f);
	rect.setPosition(sf::Vector2f(m_collider.getPosition().x, m_collider.getPosition().y + m_collider.getRadius() + m_collider.getOutlineThickness()));

	int colorR = 255 * std::clamp((healthPercent - 1.0f) * -2.0f, 0.0f, 1.0f);
	int colorG = 255 * std::clamp(healthPercent * 2.0f, 0.0f, 1.0f);

	rect.setFillColor(sf::Color(colorR, colorG, 0));

	renderer.draw(rect);
}