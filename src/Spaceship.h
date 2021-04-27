#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <queue>
#include "Order.h"
#include "Weapon.h"

class Star;
class JumpPoint;
class EffectsEmitter;
class Faction;

class Spaceship {
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
	void addOrder(const T order) { if (m_canRecieveOrders) m_orders.push_back(std::make_shared<T>(order)); }

	template <typename T>
	void addOrderFront(const T order) { if (m_canRecieveOrders) m_orders.push_front(std::make_shared<T>(order)); }

	enum class JumpState {
		TRAVEL,
		JUMPING,
		DONE
	};

	JumpState jump(JumpPoint* point);

	void setPos(sf::Vector2f pos) { m_sprite.setPosition(pos); }

	sf::Vector2f getPos() const { return m_sprite.getPosition(); }

	std::vector<Spaceship*> findEnemies();

	int getAllegiance() const { return m_allegiance; }

	void fireAt(Spaceship* target, int weaponIdx);

	sf::CircleShape& getCollider() { return m_collider; }

	void takeDamage(float damage) { m_health -= damage; }

	bool isDead() { return m_dead; }

	Star* getCurrentStar() { return m_currentStar; }

	void smartFireAt(Spaceship* target, int weaponIdx);

	sf::Vector2f getVelocity() { return m_velocity; }

	void orbit(sf::Vector2f pos);

	float getHealth() { return m_health; }

	void captureCurrentStar(Faction* faction);

	void kill() { m_health = 0.0f; }

	void attackRandomEnemy(std::vector<Spaceship*>& enemies, bool urgent = false);

	std::vector<Weapon>& getWeapons() { return m_weapons; }

	void addWeapon(Weapon weapon) { m_weapons.push_back(weapon); }

	void onSelected();

	void onDeselected();

	bool isSelected() { return m_selected; }

	void clearOrders() { m_orders.clear(); }

private:
	void init(const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color);

	sf::Sprite m_sprite;

	float m_rotationSpeed;

	float m_facingAngle = 90.0f;

	sf::Vector2f m_velocity = sf::Vector2f(0.0f, 0.0f);

	Star* m_currentStar;

	std::deque<std::shared_ptr<Order>> m_orders;

	float m_percentJumpDriveCharged = 0.0f;

	std::vector<Weapon> m_weapons;

	sf::CircleShape m_collider;

	int m_allegiance;

	float m_health;

	bool m_dead = false;

	float m_mass; // kg

	float m_maxAcceleration;

	bool m_selected = false;

	bool m_canRecieveOrders = true;
};

