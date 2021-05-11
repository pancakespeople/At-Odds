#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include "Projectile.h"

class Spaceship;
class Star;
class Unit;

class Weapon {
public:
	enum class WEAPON_TYPE {
		LASER_GUN,
		GAUSS_CANNON,
		MACHINE_GUN,
		LONG_RANGE_LASER_GUN,
		LONG_RANGE_MACHINE_GUN,
		CONSTRUCTION_GUN
	};
	
	static const std::unordered_map<WEAPON_TYPE, std::string> weaponSounds;

	Weapon(WEAPON_TYPE type);

	void fireAtAngle(const Unit* source, float angleDegrees, Star* star);

	void fireAt(const Unit* source, const sf::Vector2f& target, Star* star);

	void update();

	bool isOnCooldown();

	float getRange() { return m_projectile.getRange(); }
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& m_cooldownPercent;
		archive& m_cooldownRecovery;
		archive& m_accuracy;
		archive& m_numProjectiles;
		archive& m_projectile;
		archive& m_type;
	}

	Weapon() {}
	
	float m_cooldownPercent = 0.0f;
	float m_cooldownRecovery = 1.0f;
	float m_accuracy = 1.0f;

	int m_numProjectiles = 1;

	Projectile m_projectile;

	WEAPON_TYPE m_type;
};

