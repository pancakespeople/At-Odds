#include "gamepch.h"

#include "Weapon.h"
#include "Spaceship.h"
#include "Star.h"
#include "Debug.h"
#include "Math.h"
#include "Sounds.h"
#include "Random.h"
#include "TOMLCache.h"

Weapon::Weapon(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/weapons.toml");

	assert(table.contains(type));

	m_projectile = Projectile(table[type]["projectile"].value_or(""));

	m_soundPath = table[type]["sound"].value_or("");

	m_cooldownRecovery = table[type]["cooldownRecovery"].value_or(1.0f);
	m_accuracy = table[type]["accuracy"].value_or(1.0f);
	m_baseSoundCooldown = table[type]["baseSoundCooldown"].value_or(0);
	m_numProjectiles = table[type]["numProjectiles"].value_or(1);
}

void Weapon::fireAtAngle(sf::Vector2f sourcePos, int allegiance, float angleDegrees, Star* star) {
	if (isOnCooldown()) return;
	
	float variability = (1.0f - m_accuracy) * 100.0f;
	
	for (int i = 0; i < m_numProjectiles; i++) {
		float angleVary = Random::randFloat(-variability, variability);

		m_projectile.setPos(sourcePos);
		m_projectile.setRotation(angleDegrees + angleVary);
		m_projectile.setAllegiance(allegiance);
		star->addProjectile(m_projectile);
	}

	playFireSound(sourcePos, star);

	m_cooldownPercent = 100.0f;
}

void Weapon::update() {
	if (m_cooldownPercent > 0.0f) {
		m_cooldownPercent -= m_cooldownRecovery;
	}

	if (m_cooldownPercent < 0.0f) {
		m_cooldownPercent = 0.0f;
	}

	if (m_soundCooldown > 0) {
		m_soundCooldown--;
	}
}

bool Weapon::isOnCooldown() {
	if (m_cooldownPercent > 0.0f) {
		return true;
	}
	else {
		return false;
	}
}

void Weapon::fireAt(sf::Vector2f sourcePos, int allegiance, const sf::Vector2f& target, Star* star) {
	float angle = Math::angleBetween(sourcePos, target);
	fireAtAngle(sourcePos, allegiance, angle, star);
}

void Weapon::playFireSound(sf::Vector2f sourcePos, Star* star) {
	if (m_soundPath != "" && m_soundCooldown == 0) {
		Sounds::playSoundLocal(m_soundPath, star, sourcePos, 25.0f, 1.0f + Random::randFloat(-0.5f, 0.5f));
		m_soundCooldown = m_baseSoundCooldown;
	}
}

void Weapon::fireAtNearestEnemyCombatShip(sf::Vector2f sourcePos, int allegiance, Star* star) {
	for (auto& ship : star->getSpaceships()) {
		if (ship->getAllegiance() != allegiance && !ship->isCivilian()) {
			if (Math::distance(sourcePos, ship->getPos()) < getRange()) {
				fireAt(sourcePos, allegiance, ship->getPos(), star);
				return;
			}
		}
	}
}