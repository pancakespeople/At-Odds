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

	void draw(Renderer& renderer, const sf::RenderWindow& window);
	void setPos(sf::Vector2f pos) { m_pos = pos; }
	void jumpShipThrough(Spaceship* ship, Star* currentStar);
	void reinitAfterLoad(Star* star);
	void onClick(GameState& state, sf::Vector2f releaseMouseWorldPos, sf::Vector2f pressMouseWorldPos);

	bool isOutgoing() { return m_isOutgoing; }
	bool isPointInRadius(sf::Vector2f point);
	bool isMouseInRadius(const sf::RenderWindow& window, const Renderer& renderer);

	float getRadius() { return 256.0f; }

	sf::Vector2f getPos() { return m_pos; }

	Hyperlane* getHyperlane() { return m_hyperlane; }
	JumpPoint* getConnectedJumpPoint();
	Star* getConnectedOtherStar();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Identifiable>(*this);
		archive & m_pos;
		archive & m_trail;
		archive & m_hyperlaneID;
		archive & m_isOutgoing;
		archive & m_shaderSeed;
	}

	JumpPoint() {}
	
	sf::Vector2f m_pos;
	sf::RectangleShape m_trail;
	sf::Clock m_rotationClock;
	
	bool m_isOutgoing = false;
	float m_shaderSeed = 1.0f;

	Hyperlane* m_hyperlane = nullptr;
	uint32_t m_hyperlaneID = 0;
};

