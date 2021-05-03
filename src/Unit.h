#pragma once
#include <SFML/Graphics.hpp>

#include "Collider.h"
#include "Weapon.h"

class Star;

class Unit {
public:
	float getHealth() const { return m_health; }

	bool isDead() const { return m_dead; }

	sf::Vector2f getPos() const { return m_collider.getPosition(); }

	void takeDamage(float damage) { m_health -= damage; }

	const Collider& getCollider() const { return m_collider; }

	int getAllegiance() const { return m_allegiance; }

	void kill() { m_health = 0.0f; }

	Star* getCurrentStar() { return m_currentStar; }

	sf::Vector2f getVelocity() { return m_velocity; }

	std::vector<Weapon>& getWeapons() { return m_weapons; }

	void addWeapon(Weapon weapon) { m_weapons.push_back(weapon); }

	std::vector<Spaceship*> findEnemyShips();

	float getLongestWeaponRange();

	void updateWeapons();

protected:
	Star* m_currentStar = nullptr;

	Collider m_collider;
	sf::Vector2f m_velocity;
	std::vector<Weapon> m_weapons;

	float m_health = 100.0f;
	bool m_dead = false;
	int m_allegiance = 0;
};

