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
		CLAIM_SHIP
	};

	Spaceship(SPACESHIP_TYPE type, const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color);
	
	void draw(sf::RenderWindow& window, EffectsEmitter& emitter);

	// Returns true if angle equals the direction the ship is facing, otherwise rotates the ship based on its rotation speed
	bool rotateTo(float angleDegrees);

	void accelerate(float amount);

	void update();

	// Returns degrees
	float angleTo(const sf::Vector2f& pos);

	bool flyTo(const sf::Vector2f& pos);

	float getSpeed();

	void keepSpeed(float speed);

	template <typename T>
	void addOrder(const T order) { if (m_canReceiveOrders) m_orders.push_back(std::make_shared<T>(order)); }

	template <typename T>
	void addOrderFront(const T order) { if (m_canReceiveOrders) m_orders.push_front(std::make_shared<T>(order)); }

	enum class JumpState {
		TRAVEL,
		JUMPING,
		DONE
	};

	JumpState jump(JumpPoint* point);

	void setPos(sf::Vector2f pos) { m_sprite.setPosition(pos); }

	sf::Vector2f getPos() const { return m_sprite.getPosition(); }

	std::vector<Spaceship*> findEnemies();

	void fireAt(Spaceship* target, int weaponIdx);

	Star* getCurrentStar() { return m_currentStar; }

	void smartFireAt(Unit* target, int weaponIdx);

	void orbit(sf::Vector2f pos);

	void captureCurrentStar(Faction* faction);

	void attackRandomEnemy(std::vector<Spaceship*>& enemies, bool urgent = false);

	void onSelected();

	void onDeselected();

	bool isSelected() { return m_selected; }

	void clearOrders() { m_orders.clear(); }

	std::vector<Building*> findEnemyBuildings();

	void attackRandomEnemyBuilding(std::vector<Building*>& enemyBuildings);

private:
	void init(const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color);

	sf::Sprite m_sprite;
	
	std::deque<std::shared_ptr<Order>> m_orders;
	
	float m_mass; // kg
	float m_maxAcceleration;
	float m_percentJumpDriveCharged = 0.0f;
	float m_facingAngle = 90.0f;
	float m_rotationSpeed;

	bool m_selected = false;
	bool m_canReceiveOrders = true;
};

