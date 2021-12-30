#include "gamepch.h"

#include "Weapon.h"
#include "Spaceship.h"
#include "Star.h"
#include "Debug.h"
#include "Math.h"
#include "Sounds.h"
#include "Random.h"
#include "TOMLCache.h"
#include "Renderer.h"

void laserAnimation(Renderer& renderer, sf::Vector2f sourcePos, sf::Vector2f endPos, float step) {
	float angle = Math::angleBetween(sourcePos, endPos);
	sf::RectangleShape shape;

	shape.setRotation(-angle);
	shape.setPosition(sourcePos);
	shape.setSize(sf::Vector2f(Math::distance(sourcePos, endPos), 25.0f));
	shape.setFillColor(sf::Color(255, 0, 0, 255 * (1.0 / step)));

	renderer.draw(shape);
}

const std::unordered_map<std::string, std::function<void(Renderer& renderer, sf::Vector2f sourcePos, sf::Vector2f endPos, float step)>> fireAnimationFunctions = {
	{"laserAnimation", &laserAnimation},
};

Weapon::Weapon(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/weapons.toml");

	assert(table.contains(type));

	m_type = type;

	m_projectile = Projectile(table[type]["projectile"].value_or(""));

	m_cooldownRecovery = table[type]["cooldownRecovery"].value_or(1.0f);
	m_accuracy = table[type]["accuracy"].value_or(1.0f);
	m_baseSoundCooldown = table[type]["baseSoundCooldown"].value_or(0);
	m_numProjectiles = table[type]["numProjectiles"].value_or(1);
	m_fireAnimation = table[type]["fireAnimation"].value_or("");
	m_instaHit = table[type]["instaHit"].value_or(false);
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
	if (getSoundPath() != "" && m_soundCooldown == 0) {
		Sounds::playSoundLocal(getSoundPath(), star, sourcePos, 25.0f, 1.0f + Random::randFloat(-0.5f, 0.5f));
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

void Weapon::instaHitFireAt(sf::Vector2f sourcePos, Unit* target, Star* star) {
	if (!isOnCooldown()) {
		target->takeDamage(getDamage());
		playFireSound(sourcePos, star);

		m_lastFireLocation = target->getPos();
		m_cooldownPercent = 100.0f;
	}
}

std::string Weapon::getSoundPath() const {
	const toml::table& table = TOMLCache::getTable("data/objects/weapons.toml");
	return table[m_type]["sound"].value_or("");
}

void Weapon::drawFireAnimation(Renderer& renderer, Unit* unit) {
	if (isOnCooldown()) {
		if (m_fireAnimation != "") {
			fireAnimationFunctions.at(m_fireAnimation)(renderer, unit->getPos(), m_lastFireLocation, (100.0f - m_cooldownPercent) / 100.0f);
		}
	}
}