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
	Weapon(const std::string& type);

	void fireAtAngle(const Unit* source, float angleDegrees, Star* star);
	void fireAt(const Unit* source, const sf::Vector2f& target, Star* star);
	void update();
	void setAccuracy(float accuracy) { m_accuracy = accuracy; }
	void triggerCooldown() { m_cooldownPercent = 100.0f; }
	void playFireSound(const Unit* source, Star* star);

	bool isOnCooldown();

	float getRange() const { return m_projectile.getRange(); }
	float getAccuracy() const { return m_accuracy; }
	float getDamage() const { return m_projectile.getDamage(); }

	const Projectile& getProjectile() const { return m_projectile; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_cooldownPercent;
		archive & m_cooldownRecovery;
		archive & m_accuracy;
		archive & m_numProjectiles;
		archive & m_soundCooldown;
		archive & m_baseSoundCooldown;
		archive & m_soundPath;
		archive & m_projectile;
	}

	Weapon() {}
	
	float m_cooldownPercent = 0.0f;
	float m_cooldownRecovery = 1.0f;
	float m_accuracy = 1.0f;

	int m_numProjectiles = 1;
	int m_soundCooldown = 0;
	int m_baseSoundCooldown = 0;

	std::string m_soundPath;

	Projectile m_projectile;
};

