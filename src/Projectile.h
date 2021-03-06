#pragma once
#include <SFML/Graphics.hpp>

#include "Collider.h"
#include "Animation.h"

class Star;
class Renderer;

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
	
	void update(Star* star);
	void draw(Renderer& renderer);
	void kill() { m_life = 0.0f; }
	void setPos(const sf::Vector2f& pos);
	void setRotation(float angleDegrees);
	void setAllegiance(int allegiance);
	void setDamage(float damage) { m_damage = damage; }
	void onDeath(Star* star);
	void setLife(float life) { m_life = life; }
	void setSpeed(float speed) { m_speed = speed; }

	float getDamage() const { return m_damage; }
	float getRange() const { return m_life * m_speed; }
	float getLife() const { return m_life; }
	float getSpeed() const { return m_speed; }

	bool isDead() const { return m_life <= 0.0f; }
	bool isCollidingWith(const Collider& collider);
	bool diesOnCollision() const { return m_diesOnCollision; }
	bool canOrbitallyBombard() const { return m_orbitalBombardment; }
	bool canInvadePlanets() const { return m_planetaryInvasion; }

	int getAllegiance() { return m_allegiance; }

	sf::Vector2f getPos() { return m_sprite.getPosition(); }

	const Collider& getCollider();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_angle;
		archive & m_sprite;
		archive & m_life;
		archive & m_speed;
		archive & m_allegiance;
		archive & m_damage;
		archive & m_collider;
		archive & m_onDeathScript;
		archive & m_diesOnCollision;
		archive & m_orbitalBombardment;
		archive & m_planetaryInvasion;
	}
	
	void init(const sf::Vector2f& pos, float angleDegrees, int allegiance);

	float m_angle; // Degrees
	sf::Sprite m_sprite;
	float m_life;
	float m_speed;
	int m_allegiance;
	float m_damage;
	Collider m_collider;
	std::string m_onDeathScript;
	bool m_diesOnCollision = true;
	bool m_orbitalBombardment = false;
	bool m_planetaryInvasion = false;
};
