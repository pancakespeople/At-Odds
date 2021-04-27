#pragma once
#include <SFML/Graphics.hpp>

#include "EffectsEmitter.h"

class Hyperlane;
class Spaceship;
class Star;

class JumpPoint {
public:
	JumpPoint(sf::Vector2f pos, float angleRadians, Hyperlane* hyperlane, bool isOutgoing);

	void draw(sf::RenderWindow& window, EffectsEmitter& emitter);

	sf::Vector2f getPos() { return m_sprite.getPosition(); }

	float getRadius() { return m_sprite.getTextureRect().width / 2.0f; }

	void setPos(sf::Vector2f pos) { m_sprite.setPosition(pos); }

	Hyperlane* getHyperlane() { return m_hyperlane; }

	bool isOutgoing() { return m_isOutgoing; }

	JumpPoint* getConnectedJumpPoint();

	void jumpShipThrough(Spaceship* ship, Star* currentStar);

	Star* getConnectedOtherStar();

	bool isPointInRadius(sf::Vector2f point);

private:
	sf::Sprite m_sprite;
	sf::RectangleShape m_trail;
	Hyperlane* m_hyperlane;
	bool m_isOutgoing;
	sf::Clock m_rotationClock;
};

