#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include "Projectile.h"

class Spaceship;
class Star;

class Weapon {
public:
	enum class WEAPON_TYPE {
		LASER_GUN,
		GAUSS_CANNON,
		MACHINE_GUN
	};
	
	static const std::unordered_map<WEAPON_TYPE, std::string> weaponSounds;

	Weapon(WEAPON_TYPE type);

	void fireAtAngle(const Spaceship* source, float angleDegrees, Star* star);

	void fireAt(const Spaceship* source, const sf::Vector2f& target, Star* star);

	void update();

	bool isOnCooldown();

	float getRange() { return m_projectile.getRange(); }
private:
	float m_cooldownPercent = 0.0f;

	float m_cooldownRecovery = 1.0f;

	Projectile m_projectile;

	WEAPON_TYPE m_type;

	float m_accuracy = 1.0f;
};

