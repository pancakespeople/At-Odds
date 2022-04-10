#pragma once
#include "Orbit.h"
#include "Identifiable.h"

class Renderer;
class Faction;
class Star;

class Asteroid : public Identifiable {
public:
	Asteroid(sf::Vector2f pos, sf::Vector2f starPos);

	void draw(Renderer& renderer, const Star& star);
	void update();
	void mineAsteroid(Faction& faction, float amount);
	void setSelected(bool selected) { m_selected = selected; }

	sf::Vector2f getPos() const { return m_sprite.getPosition(); }
	const Orbit& getOrbit() const { return m_orbit; }

	std::string getResource() { return m_resource; }
	float getRadius() const {return m_sprite.getLocalBounds().width * 1.25f; }
	float getResourceCount() const { return m_resourceCount; }
	bool isDestructing() const { return m_destructionTimer > 0; }
	bool isDead() const { return m_destructionTimer >= 10; }
	bool isSelected() const { return m_selected; }
	bool contains(sf::Vector2f point) const;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Identifiable>(*this);
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
	bool m_selected = false;
};