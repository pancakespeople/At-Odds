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
	
	Projectile(const std::string& type);

	Projectile();
	
	void update();
	void draw(sf::RenderWindow& window);
	void kill() { m_life = 0.0f; }
	void setPos(const sf::Vector2f& pos);
	void setRotation(float angleDegrees);
	void setAllegiance(int allegiance);

	float getDamage() { return m_damage; }
	float getRange() { return m_life * m_speed; }
	float getLife() { return m_life; }

	bool isDead() { return m_life <= 0.0f; }
	bool isCollidingWith(const Collider& collider);

	int getAllegiance() { return m_allegiance; }

	sf::Vector2f getPos() { if (m_usesSprite) return m_sprite.getPosition(); else return m_shape.getPosition(); }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& m_angle;
		archive& m_shape;
		archive& m_sprite;
		archive& m_life;
		archive& m_speed;
		archive& m_allegiance;
		archive& m_damage;
		archive& m_usesSprite;
	}
	
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

