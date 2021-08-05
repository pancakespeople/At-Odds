#pragma once
#include <SFML/Graphics.hpp>

class Star;

#include "Star.h"
#include "Identifiable.h"

#define HYPERLANE_THICKNESS 2.5f

class Hyperlane : public Identifiable {
public:
	Hyperlane(Star* begin, Star* end);

	void draw(sf::RenderWindow& window);
	void setColor(sf::Color color) { m_shape.setFillColor(color); }
	void enablePathEffect();
	void reinitAfterLoad(Constellation* constellation);

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
		archive & m_pathEffectShape;
		archive & m_beginStarID;
		archive & m_endStarID;
		archive & m_angleDegrees;
		archive & m_pathEffectEnabled;
	}

	Hyperlane() {}
	
	sf::RectangleShape m_shape;
	sf::RectangleShape m_pathEffectShape;
	
	Star* m_beginStar = nullptr;
	Star* m_endStar = nullptr;

	uint32_t m_beginStarID = 0;
	uint32_t m_endStarID = 0;

	float m_angleDegrees = 0;
	bool m_pathEffectEnabled = false;
	sf::Clock m_pathEffectClock;
};

