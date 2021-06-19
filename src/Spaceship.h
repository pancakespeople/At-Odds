#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <queue>
#include "Order.h"
#include "Weapon.h"
#include "Collider.h"
#include "Unit.h"

class Star;
class JumpPoint;
class EffectsEmitter;
class Faction;
class Building;

class Spaceship : public Unit {
public:
	enum class SPACESHIP_TYPE {
		FRIGATE_1,
		DESTROYER_1,
		CLAIM_SHIP,
		CONSTRUCTION_SHIP,
		FIGHTER,
		SPACE_BUS
	};
	
	enum class JumpState {
		TRAVEL,
		JUMPING,
		DONE
	};

	Spaceship(SPACESHIP_TYPE type, const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color);
	Spaceship(const Spaceship& old) = delete;
	
	void draw(sf::RenderWindow& window, EffectsEmitter& emitter);
	void accelerate(float amount);
	void update(Star* currentStar);
	void keepSpeed(float speed);
	void smartFireAt(Unit* target, int weaponIdx);
	void orbit(const sf::Vector2f& pos);
	void captureCurrentStar(Faction* faction);
	void attackRandomEnemy(std::vector<Spaceship*>& enemies, bool urgent = false);
	void onSelected();
	void onDeselected();
	void setPos(sf::Vector2f pos) { m_sprite.setPosition(pos); }
	void clearOrders() { m_orders.clear(); }
	void attackRandomEnemyBuilding(std::vector<Building*>& enemyBuildings);
	void fireAt(Spaceship* target, int weaponIdx);
	void disable() { m_disabled = true; }
	void enable() { m_disabled = false; }
	void setSilentDeath(bool flag) { m_diesSilently = flag; }
	void setCanReceiveOrders(bool flag) { m_canReceiveOrders = flag; }

	// Returns true if angle equals the direction the ship is facing, otherwise rotates the ship based on its rotation speed
	bool rotateTo(float angleDegrees);
	bool flyTo(const sf::Vector2f& pos);
	bool isSelected() { return m_selected; }
	bool isDisabled() { return m_disabled; }
	bool canReceiveOrders() { return m_canReceiveOrders; }

	// Returns degrees
	float angleTo(const sf::Vector2f& pos);
	float getSpeed();
	float getConstructionSpeed() const { return m_constructionSpeed; }

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
		archive & m_disabled;
		archive & m_fighterAI;
		archive & m_diesSilently;
	}

	Spaceship() {}
	
	void init(const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color);

	sf::Sprite m_sprite;
	
	std::deque<std::unique_ptr<Order>> m_orders;
	
	float m_mass; // kg
	float m_maxAcceleration;
	float m_percentJumpDriveCharged = 0.0f;
	float m_facingAngle = 90.0f;
	float m_rotationSpeed;
	float m_constructionSpeed = 0.0f;

	bool m_selected = false;
	bool m_canReceiveOrders = true;
	bool m_disabled = false;
	bool m_fighterAI = false;
	bool m_diesSilently = false;
};

