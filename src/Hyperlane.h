#pragma once
#include <SFML/Graphics.hpp>

class Star;

#include "Star.h"

#define HYPERLANE_THICKNESS 2.5f

class Hyperlane {
public:
	Hyperlane(Star* begin, Star* end);

	void draw(sf::RenderWindow& window);

	Star* getBeginStar() { return m_beginStar; }

	Star* getEndStar() { return m_endStar; }

	void setColor(sf::Color color) { m_shape.setFillColor(color); }

	float getAngleDegrees() { return m_angleDegrees; }

	float getAngleRadians() { return m_angleDegrees * (3.14159 / 180); }

	void enablePathEffect();

private:
	sf::RectangleShape m_shape;
	sf::RectangleShape m_pathEffectShape;
	Star* m_beginStar;
	Star* m_endStar;
	float m_angleDegrees;
	bool m_pathEffectEnabled = false;
	sf::Clock m_pathEffectClock;
};

