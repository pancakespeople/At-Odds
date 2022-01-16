#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include "Projectile.h"

class Spaceship;
class Star;
class Unit;
class Renderer;

class Weapon {
public:
	Weapon(const std::string& type);

	void fireAtAngle(sf::Vector2f sourcePos, int allegiance, float angleDegrees, Star* star);
	void fireAt(sf::Vector2f sourcePos, int allegiance, const sf::Vector2f& target, Star* star);
	void fireAtNearestEnemyCombatShip(sf::Vector2f sourcePos, int allegiance, Star* star);
	void update();
	void setAccuracy(float accuracy) { m_accuracy = accuracy; }
	void triggerCooldown() { m_cooldownPercent = 100.0f; }
	void playFireSound(sf::Vector2f sourcePos, Star* star);
	void drawFireAnimation(Renderer& renderer, Unit* unit);
	void setDamage(float damage) { m_projectile.setDamage(damage); }
	void setUpgradeLevel(int level);
	
	// Insta hit makes the projectile hit the target always, instantly
	void instaHitFireAt(sf::Vector2f sourcePos, Unit* target, Star* star);

	bool isOnCooldown();
	bool isInstaHit() { return m_instaHit; }

	float getRange() const { return m_projectile.getRange(); }
	float getAccuracy() const { return m_accuracy; }
	float getDamage() const { return m_projectile.getDamage(); }

	const Projectile& getProjectile() const { return m_projectile; }

	std::string getSoundPath() const;

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
		archive & m_type;
		archive & m_projectile;
		archive & m_instaHit;
		archive & m_lastFireLocation;
		archive & m_fireAnimation;
	}

	Weapon() {}
	
	float m_cooldownPercent = 0.0f;
	float m_cooldownRecovery = 1.0f;
	float m_accuracy = 1.0f;

	int m_numProjectiles = 1;
	int m_soundCooldown = 0;
	int m_baseSoundCooldown = 0;

	std::string m_type;
	std::string m_fireAnimation;

	Projectile m_projectile;

	bool m_instaHit = false;

	sf::Vector2f m_lastFireLocation;
};

