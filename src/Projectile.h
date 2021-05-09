#pragma once
#include <SFML/Graphics.hpp>

#include "Collider.h"

class Star;

class Projectile {
public:
	enum class PROJECTILE_TYPE {
		LASER,
		GAUSS,
		LIGHT_BALLISTIC,
		LONG_RANGE_LASER,
		LONG_RANGE_LIGHT_BALLISTIC,
		CONSTRUCTION
	};
	
	Projectile(PROJECTILE_TYPE type);

	Projectile();
	
	void update();

	void draw(sf::RenderWindow& window);

	float getLife() { return m_life; }

	bool isDead() { return m_life <= 0.0f; }

	bool isCollidingWith(const Collider& collider);

	void kill() { m_life = 0.0f; }

	int getAllegiance() { return m_allegiance; }

	float getDamage() { return m_damage; }

	float getRange() { return m_life * m_speed; }

	void setPos(const sf::Vector2f& pos);

	void setRotation(float angleDegrees);

	void setAllegiance(int allegiance);

private:
	void init(const sf::Vector2f& pos, float angleDegrees, int allegiance);

	float m_angle; // Degrees
	sf::RectangleShape m_shape;
	sf::Sprite m_sprite;
	float m_life;
	float m_speed;
	int m_allegiance;
	float m_damage;
	bool m_usesSprite = false;
};

