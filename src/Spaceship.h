#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <queue>
#include "Order.h"
#include "Weapon.h"
#include "Collider.h"
#include "Unit.h"
#include "Planet.h"

class Star;
class JumpPoint;
class EffectsEmitter;
class Faction;
class Building;
class AllianceList;

class Spaceship : public Unit {
public:
	enum class JumpState {
		TRAVEL,
		JUMPING,
		DONE
	};

	Spaceship(const std::string& type, const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color);
	Spaceship(const Spaceship& old) = delete;
	
	void draw(Renderer& renderer, float time, sf::Vector2f mouseWorldPos);
	void accelerate(float amount);
	void update(Star* currentStar, const AllianceList& alliances, Faction* faction);
	void keepSpeed(float speed);
	void smartFireAt(Unit* target, int weaponIdx);
	void orbit(sf::Vector2f pos);
	void orbitStable(const Orbit& orbit, float orbitOffset);
	void captureCurrentStar(Faction* faction, const AllianceList& alliances);
	void attackRandomEnemy(std::vector<Spaceship*>& enemies, bool urgent = false);
	void setPos(sf::Vector2f pos) { m_sprite.setPosition(pos); }
	void clearOrders() { m_orders.clear(); }
	void attackRandomEnemyBuilding(std::vector<Building*>& enemyBuildings, bool urgent = false);
	void fireAt(Spaceship* target, int weaponIdx);
	void disable() { m_disabled = true; }
	void enable() { m_disabled = false; }
	void setSilentDeath(bool flag) { m_diesSilently = flag; }
	void setCanReceiveOrders(bool flag) { m_canReceiveOrders = flag; }
	void reinitOrdersAfterLoad(Constellation* constellation);
	void setPirate(bool pirate) { m_pirate = pirate; }
	void onDeath(Star* currentStar);
	void createThrusterParticles();
	void setAIEnabled(bool enabled) { m_AIEnabled = enabled; }

	// Returns true if angle equals the direction the ship is facing, otherwise rotates the ship based on its rotation speed
	bool rotateTo(float angleDegrees);
	bool flyTo(const sf::Vector2f& pos);
	bool isDisabled() const { return m_disabled; }
	bool canReceiveOrders() const { return m_canReceiveOrders; }
	bool canPlayerGiveOrders() const { return m_playerCanGiveOrders; }
	bool isCivilian() const { return m_civilian; }
	bool isConstructor() const { return m_constructionSpeed > 0.0f; }
	bool isHeavy() const { return m_mass >= 125000; }
	bool isPlanetAttackShip() const;
	bool hasFighterAI() const { return m_fighterAI; }
	bool isPirate() const { return m_pirate; }
	bool diesSilently() const { return m_diesSilently; }
	bool isCombatShip() const { return !isCivilian() && maxWeaponDamage() > 0.0f; }
	
	// Returns false if there is nothing to attack
	bool attack(Star* star, const AllianceList& alliances, bool urgent = false);

	// Returns degrees
	float angleTo(const sf::Vector2f& pos);
	float getSpeed();
	float getConstructionSpeed() const { return m_constructionSpeed; }
	float getMass() const { return m_mass; }
	float getFacingAngle() const { return m_facingAngle; }
	float getMaxAcceleration() const { return m_maxAcceleration; }

	int numOrders() { return m_orders.size(); }

	sf::Vector2f getPos() const { return m_sprite.getPosition(); }
	sf::Vector2f getSize() const { return sf::Vector2f(m_sprite.getTextureRect().width , m_sprite.getTextureRect().height); }
	sf::Vector2f getScale() const { return m_sprite.getScale(); }

	template <typename T>
	void addOrder(const T order) { if (m_canReceiveOrders) m_orders.push_back(std::make_unique<T>(order)); }

	template <typename T>
	void addOrderFront(const T order) { 
		if (m_canReceiveOrders) {
			m_orders.front()->executing = false;
			m_orders.push_front(std::make_unique<T>(order));
		}
	}

	JumpState jump(JumpPoint* point);

	Star* getCurrentStar() { return m_currentStar; }

	std::vector<Building*> findEnemyBuildings(const AllianceList& alliances);
	std::vector<Spaceship*> findEnemyShips(const AllianceList& alliances);
	std::vector<Spaceship*> findEnemyCombatShips(const AllianceList& alliances);
	Spaceship* findClosestEnemyCombatShip(Star* star, const AllianceList& alliances);
	Spaceship * findClosestEnemyShip(Star * star, const AllianceList& alliances);

	std::string getName();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Unit>(*this);
		archive & m_sprite;
		archive & m_orders;
		archive & m_mass;
		archive & m_maxAcceleration;
		archive & m_percentJumpDriveCharged;
		archive & m_facingAngle;
		archive & m_rotationSpeed;
		archive & m_constructionSpeed;
		archive & m_canReceiveOrders;
		archive & m_playerCanGiveOrders;
		archive & m_disabled;
		archive & m_fighterAI;
		archive & m_diesSilently;
		archive & m_civilian;
		archive & m_pirate;
		archive & m_thrusterPoints;
		archive & m_thrusterSize;
		archive & m_AIEnabled;
	}

	Spaceship() {}
	
	void init(const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color);

	sf::Sprite m_sprite;
	
	std::deque<std::unique_ptr<Order>> m_orders;
	std::vector<sf::Vector2f> m_thrusterPoints;
	
	float m_mass = 100.0; // kg
	float m_maxAcceleration = 10.0;
	float m_percentJumpDriveCharged = 0.0f;
	float m_facingAngle = 90.0f;
	float m_rotationSpeed = 1.0;
	float m_constructionSpeed = 0.0f;
	float m_thrusterSize = 100.0f;

	bool m_canReceiveOrders = true;
	bool m_playerCanGiveOrders = true;
	bool m_disabled = false;
	bool m_fighterAI = false;
	bool m_diesSilently = false;
	bool m_civilian = false;
	bool m_pirate = false;
	bool m_AIEnabled = true;
};

