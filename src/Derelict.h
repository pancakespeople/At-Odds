#pragma once
#include <SFML/Graphics.hpp>

#include "Collider.h"
#include "Faction.h"

class Star;
class Renderer;

class Derelict {
public:
	Derelict(sf::Vector2f pos);
	
	void draw(Renderer& renderer);
	void update(Star* star, std::vector<Faction>& factions);

	bool isDead() { return m_dead; }

	sf::Vector2f getPos() { return m_sprite.getPosition(); }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_sprite;
		archive & m_collider;
		archive & m_dead;
	}
	
	Derelict() {}

	sf::Sprite m_sprite;
	Collider m_collider;

	bool m_dead = false;
};