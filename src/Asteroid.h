#pragma once
#include "Orbit.h"
#include "Identifiable.h"

class Renderer;
class Faction;

class Asteroid : public Identifiable {
public:
	Asteroid(sf::Vector2f pos, sf::Vector2f starPos);

	void draw(Renderer& renderer);
	void update();

	sf::Vector2f getPos() { return m_sprite.getPosition(); }
	std::string getResource() { return m_resource; }
	float getRadius() {return m_sprite.getLocalBounds().width * 1.25f; }
	void mineAsteroid(Faction& faction, float amount);
	bool isDestructing() { return m_destructionTimer > 0; }
	bool isDead() { return m_destructionTimer >= 10; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_sprite;
		archive & m_orbit;
		archive & m_resource;
		archive & m_resourceCount;
		archive & m_destructionTimer;
	}

	Asteroid() = default;

	sf::Sprite m_sprite;
	Orbit m_orbit;
	std::string m_resource;
	float m_resourceCount = 0.0f;
	int m_destructionTimer = 0;
};