#pragma once
#include <SFML/Graphics.hpp>

#include "Collider.h"
#include "Weapon.h"
#include "SaveLoader.h"
#include "Identifiable.h"
#include "Mod.h"

class Star;
class Faction;

class Unit : public Identifiable {
public:
	Star* getCurrentStar() { return m_currentStar; }
	
	const Collider& getCollider() const { return m_collider; }

	sf::Vector2f getPos() const { return m_collider.getPosition(); }
	sf::Vector2f getVelocity() { return m_velocity; }

	std::vector<Weapon>& getWeapons() { return m_weapons; }
	std::vector<Spaceship*> findEnemyShips();
	std::vector<Unit*> findEnemyUnits();

	float getLongestWeaponRange();
	float getHealth() const { return m_health; }

	bool isDead() const { return m_dead; }
	
	void kill() { m_health = 0.0f; }
	void takeDamage(float damage) { m_health -= damage; }
	void addWeapon(Weapon weapon) { m_weapons.push_back(weapon); }
	void updateWeapons();
	void fireAllWeaponsAt(Unit* target);
	void updateMods(Star* currentStar, Faction& faction);
	
	int getAllegiance() const { return m_allegiance; }

	template <typename T>
	void addMod(const T& mod) {
		m_mods.push_back(std::make_unique<T>(mod));
	}

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Identifiable>(*this);
		archive & m_currentStar;
		archive & m_collider;
		archive & m_velocity;
		archive & m_weapons;
		archive & m_health;
		archive & m_dead;
		archive & m_allegiance;
		archive & m_mods;
	}

	Star* m_currentStar = nullptr;

	Collider m_collider;
	sf::Vector2f m_velocity;
	
	std::vector<Weapon> m_weapons;
	std::vector<std::unique_ptr<Mod>> m_mods;

	int m_allegiance = 0;
	float m_health = 100.0f;
	bool m_dead = false;
};

