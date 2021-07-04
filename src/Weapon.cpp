#include "gamepch.h"

#include "Weapon.h"
#include "Spaceship.h"
#include "Star.h"
#include "Debug.h"
#include "Math.h"
#include "Sounds.h"
#include "Random.h"

const std::unordered_map<Weapon::WEAPON_TYPE, std::string> Weapon::weaponSounds = {
	{WEAPON_TYPE::LASER_GUN, "data/sound/pew1.wav"},
	{WEAPON_TYPE::GAUSS_CANNON, "data/sound/pew2.wav" },
	{WEAPON_TYPE::MACHINE_GUN, "data/sound/gunshot3.wav"},
	{WEAPON_TYPE::LONG_RANGE_LASER_GUN, "data/sound/pew1.wav"},
	{WEAPON_TYPE::LONG_RANGE_MACHINE_GUN, "data/sound/gunshot3.wav"},
};

Weapon::Weapon(WEAPON_TYPE type) {
	switch (type) {
	case WEAPON_TYPE::LASER_GUN:
		m_projectile = Projectile(Projectile::PROJECTILE_TYPE::LASER);
		break;
	case WEAPON_TYPE::GAUSS_CANNON:
		m_projectile = Projectile(Projectile::PROJECTILE_TYPE::GAUSS);
		break;
	case WEAPON_TYPE::MACHINE_GUN:
		m_projectile = Projectile(Projectile::PROJECTILE_TYPE::LIGHT_BALLISTIC);
		m_cooldownRecovery = 5.0f;
		m_accuracy = 0.9f;
		m_baseSoundCooldown = 60;
		break;
	case WEAPON_TYPE::LONG_RANGE_LASER_GUN:
		m_projectile = Projectile(Projectile::PROJECTILE_TYPE::LONG_RANGE_LASER);
		m_cooldownRecovery = 0.5f;
		m_accuracy = 0.95f;
		break;
	case WEAPON_TYPE::LONG_RANGE_MACHINE_GUN:
		m_projectile = Projectile(Projectile::PROJECTILE_TYPE::LONG_RANGE_LIGHT_BALLISTIC);
		m_cooldownRecovery = 0.5f;
		m_accuracy = 0.95f;
		m_numProjectiles = 2;
		m_baseSoundCooldown = 60;
		break;
	case WEAPON_TYPE::CONSTRUCTION_GUN:
		m_projectile = Projectile(Projectile::PROJECTILE_TYPE::CONSTRUCTION);
		m_cooldownRecovery = 5.0f;
		break;
	default:
		DEBUG_PRINT("Invalid weapon type");
	}

	m_type = type;
}

void Weapon::fireAtAngle(const Unit* source, float angleDegrees, Star* star) {
	if (isOnCooldown()) return;
	
	float variability = (1.0f - m_accuracy) * 100.0f;
	
	for (int i = 0; i < m_numProjectiles; i++) {
		float angleVary = Random::randFloat(-variability, variability);

		m_projectile.setPos(source->getPos());
		m_projectile.setRotation(angleDegrees + angleVary);
		m_projectile.setAllegiance(source->getAllegiance());
		star->addProjectile(m_projectile);
	}

	if (weaponSounds.count(m_type) > 0 && m_soundCooldown == 0) {
		Sounds::playSoundLocal(weaponSounds.at(m_type), star, source->getPos(), 25.0f, 1.0f + Random::randFloat(-0.5f, 0.5f));
		m_soundCooldown = m_baseSoundCooldown;
	}

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

void Weapon::fireAt(const Unit* source, const sf::Vector2f& target, Star* star) {
	float angle = Math::angleBetween(source->getPos(), target);
	fireAtAngle(source, angle, star);
}