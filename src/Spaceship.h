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

class Spaceship : public Unit {
public:
	enum class JumpState {
		TRAVEL,
		JUMPING,
		DONE
	};

	Spaceship(const std::string& type, const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color);
	Spaceship(const Spaceship& old) = delete;
	
	void draw(sf::RenderWindow& window, EffectsEmitter& emitter, float time);
	void accelerate(float amount);
	void update(Star* currentStar);
	void keepSpeed(float speed);
	void smartFireAt(Unit* target, int weaponIdx);
	void orbit(const sf::Vector2f& pos);
	void captureCurrentStar(Faction* faction);
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
	
	// Returns false if there is nothing to attack
	bool attack(Star* star, bool urgent = false);

	// Returns degrees
	float angleTo(const sf::Vector2f& pos);
	float getSpeed();
	float getConstructionSpeed() const { return m_constructionSpeed; }
	float getMass() const { return m_mass; }
	float getFacingAngle() const { return m_facingAngle; }

	int numOrders() { return m_orders.size(); }

	sf::Vector2f getPos() const { return m_sprite.getPosition(); }

	template <typename T>
	void addOrder(const T order) { if (m_canReceiveOrders) m_orders.push_back(std::make_unique<T>(order)); }

	template <typename T>
	void addOrderFront(const T order) { if (m_canReceiveOrders) m_orders.push_front(std::make_unique<T>(order)); }

	JumpState jump(JumpPoint* point);

	Star* getCurrentStar() { return m_currentStar; }

	std::vector<Building*> findEnemyBuildings();
	std::vector<Spaceship*> findEnemyShips();
	std::vector<Spaceship*> findEnemyCombatShips();
	Spaceship* findClosestEnemyCombatShip(Star* star);
	Spaceship * findClosestEnemyShip(Star * star);

	std::string getName() { return m_name; }

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
		archive & m_name;
		archive & m_pirate;
	}

	Spaceship() {}
	
	void init(const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color);

	sf::Sprite m_sprite;
	
	std::deque<std::unique_ptr<Order>> m_orders;
	
	std::string m_name;

	float m_mass = 100.0; // kg
	float m_maxAcceleration = 10.0;
	float m_percentJumpDriveCharged = 0.0f;
	float m_facingAngle = 90.0f;
	float m_rotationSpeed = 1.0;
	float m_constructionSpeed = 0.0f;

	bool m_canReceiveOrders = true;
	bool m_playerCanGiveOrders = true;
	bool m_disabled = false;
	bool m_fighterAI = false;
	bool m_diesSilently = false;
	bool m_civilian = false;
	bool m_pirate = false;
};

