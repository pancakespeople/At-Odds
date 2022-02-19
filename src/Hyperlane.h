#pragma once
#include <SFML/Graphics.hpp>

class Star;
class Renderer;

#include "Star.h"
#include "Identifiable.h"

#define HYPERLANE_THICKNESS 2.5f

class Hyperlane : public Identifiable {
public:
	Hyperlane(Star* begin, Star* end);

	void draw(Renderer& renderer, int playerFaction, const AllianceList& alliances);
	void setColor(sf::Color color) { m_shape.setFillColor(color); }
	void enablePathEffect();
	void reinitAfterLoad(Constellation* constellation);
	void addJumpEffect(const sf::Color& color, const Star* star);
	void onJump();

	Star* getBeginStar() { return m_beginStar; }
	Star* getEndStar() { return m_endStar; }

	float getAngleDegrees() { return m_angleDegrees; }
	float getAngleRadians() { return m_angleDegrees * (3.14159 / 180); }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Identifiable>(*this);
		archive & m_shape;
		archive & m_beginStarID;
		archive & m_endStarID;
		archive & m_angleDegrees;
		archive & m_pathEffectEnabled;
		archive & m_vertices;
	}

	Hyperlane() {}
	
	sf::RectangleShape m_shape;
	
	Star* m_beginStar = nullptr;
	Star* m_endStar = nullptr;

	uint32_t m_beginStarID = 0;
	uint32_t m_endStarID = 0;

	float m_angleDegrees = 0;
	bool m_pathEffectEnabled = false;
	
	sf::Clock m_pathEffectClock;

	struct JumpEffect {
		sf::Clock clock;
		bool beginToEndJumped = false;
		sf::Color color;
	};

	std::vector<sf::Vertex> m_vertices;
	std::deque<JumpEffect> m_jumpEffects;
};

