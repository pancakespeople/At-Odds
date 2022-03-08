#pragma once
#include <SFML/Graphics.hpp>

#include "Collider.h"
#include "Weapon.h"
#include "SaveLoader.h"
#include "Identifiable.h"
#include "Mod.h"

class Star;
class Faction;
class EffectsEmitter;
class Constellation;
class Renderer;
class AllianceList;

class Unit : public Identifiable {
public:
	Star* getCurrentStar() { return m_currentStar; }
	
	const Collider& getCollider() const { return m_collider; }

	sf::Vector2f getPos() const { return m_collider.getPosition(); }
	sf::Vector2f getVelocity() const { return m_velocity; }

	sf::Color getFactionColor() const { return m_collider.getColor(); }

	std::vector<Weapon>& getWeapons() { return m_weapons; }
	std::vector<Spaceship*> findEnemyShips(const AllianceList& alliances);
	std::vector<Unit*> findEnemyUnits(const AllianceList& alliances);
	std::vector<std::unique_ptr<Mod>>& getMods() { return m_mods; }

	float getLongestWeaponRange();
	float getHealth() const { return m_health; }
	float maxWeaponDamage() const;

	bool isDead() const { return m_dead; }
	bool isSelected() const { return m_selected; }
	bool hasWeapon(const std::string& type) const;
	
	void kill() { m_health = 0.0f; }
	void takeDamage(float damage) { m_health -= damage; }
	void addWeapon(Weapon weapon);
	void updateWeapons();
	void fireAllWeaponsAt(Unit* target);
	void updateMods(Star& currentStar, Faction* faction, const AllianceList& alliances);
	void enableAllMods();
	void onDeath(Star* currentStar);
	void openModGUI(tgui::ChildWindow::Ptr window, Faction* faction);
	void reinitAfterLoad(Star* star) { m_currentStar = star; }
	void addVelocity(sf::Vector2f vel) { if (!m_stationary) m_velocity += vel; }
	void onSelected() { m_selected = true; }
	void onDeselected() { m_selected = false; }
	void drawSelectionCircle(Renderer& renderer);
	void drawHealthBar(Renderer& renderer);
	
	int getAllegiance() const { return m_allegiance; }

	template <typename T>
	void addMod(const T& mod) {
		m_mods.push_back(std::make_unique<T>(mod));
	}

	template <typename T>
	T* getMod() {
		for (auto& mod : m_mods) {
			if (dynamic_cast<T*>(mod.get()) != nullptr) {
				return dynamic_cast<T*>(mod.get());
			}
		}
		return nullptr;
	}

	Weapon* getWeapon(const std::string& type);

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Identifiable>(*this);
		archive & m_collider;
		archive & m_velocity;
		archive & m_weapons;
		archive & m_health;
		archive & m_dead;
		archive & m_allegiance;
		archive & m_mods;
		archive & m_maxHealth;
		archive & m_stationary;
		archive & m_type;
	}

	Star* m_currentStar = nullptr;

	Collider m_collider;
	sf::Vector2f m_velocity;
	
	std::vector<Weapon> m_weapons;
	std::vector<std::unique_ptr<Mod>> m_mods;
	std::string m_type;

	int m_allegiance = 0;
	
	float m_health = 100.0f;
	float m_maxHealth = 100.0f;
	
	bool m_dead = false;
	bool m_selected = false;
	bool m_stationary = false;
};

