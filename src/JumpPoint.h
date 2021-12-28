#pragma once
#include <SFML/Graphics.hpp>

#include "EffectsEmitter.h"
#include "Identifiable.h"

class Hyperlane;
class Spaceship;
class Star;

class JumpPoint : public Identifiable {
public:
	JumpPoint(sf::Vector2f pos, float angleRadians, Hyperlane* hyperlane, bool isOutgoing);

	void draw(sf::RenderWindow& window, EffectsEmitter& emitter);
	void setPos(sf::Vector2f pos) { m_sprite.setPosition(pos); }
	void jumpShipThrough(Spaceship* ship, Star* currentStar);
	void reinitAfterLoad(Star* star);
	void onClick(GameState& state, sf::Vector2f releaseMouseWorldPos, sf::Vector2f pressMouseWorldPos);

	bool isOutgoing() { return m_isOutgoing; }
	bool isPointInRadius(sf::Vector2f point);
	bool isMouseInRadius(const sf::RenderWindow& window);

	float getRadius() { return m_sprite.getTextureRect().width / 2.0f; }

	sf::Vector2f getPos() { return m_sprite.getPosition(); }

	Hyperlane* getHyperlane() { return m_hyperlane; }
	JumpPoint* getConnectedJumpPoint();
	Star* getConnectedOtherStar();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Identifiable>(*this);
		archive & m_sprite;
		archive & m_trail;
		archive & m_hyperlaneID;
		archive & m_isOutgoing;
	}

	JumpPoint() {}
	
	sf::Sprite m_sprite;
	sf::RectangleShape m_trail;
	Hyperlane* m_hyperlane = nullptr;
	bool m_isOutgoing = false;
	sf::Clock m_rotationClock;
	uint32_t m_hyperlaneID = 0;
};

