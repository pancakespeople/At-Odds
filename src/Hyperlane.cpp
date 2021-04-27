#include "gamepch.h"

#include "Hyperlane.h"
#include "Debug.h"

Hyperlane::Hyperlane(Star* begin, Star* end) : m_beginStar(begin), m_endStar(end) {
	setColor(sf::Color(255, 255, 255, 100));
	
	sf::Vector2f beginpos = begin->getPos();
	sf::Vector2f endpos = end->getPos();

	beginpos.x += begin->getRadius();
	beginpos.y += begin->getRadius();
	endpos.x += end->getRadius();
	endpos.y += end->getRadius();

	float distX = endpos.x - beginpos.x;
	float distY = endpos.y - beginpos.y;
	float dist = std::sqrt(std::pow(distX, 2) + std::pow(distY, 2));

	float angleDegrees = std::atan(distY / distX) * (180 / 3.14159);

	m_shape.setSize(sf::Vector2f(dist, HYPERLANE_THICKNESS));
	
	if (beginpos.x < endpos.x) {
		m_shape.rotate(angleDegrees);
		m_angleDegrees = angleDegrees;
	}
	else {
		m_shape.rotate(angleDegrees - 180);
		m_angleDegrees = angleDegrees - 180;
	}

	// Nicer position
	sf::Vector2f hyPos = beginpos;
	hyPos.x += std::sin(m_angleDegrees * (3.14159 / 180)) * (HYPERLANE_THICKNESS / 2.0f);
	hyPos.y -= std::cos(m_angleDegrees * (3.14159 / 180)) * (HYPERLANE_THICKNESS / 2.0f);
	
	m_shape.setPosition(hyPos);

	begin->connectHyperlane(this);
	end->connectHyperlane(this);
}

void Hyperlane::draw(sf::RenderWindow& window) {
	window.draw(m_shape);

	if (m_pathEffectEnabled) {
		window.draw(m_pathEffectShape);
		
		sf::Color newColor = m_pathEffectShape.getFillColor();
		
		float time = m_pathEffectClock.getElapsedTime().asSeconds() * 60.0f;
		m_pathEffectClock.restart();

		if (newColor.a - time <= 0) {
			m_pathEffectEnabled = false;
		}
		else {
			newColor.a -= time;
			m_pathEffectShape.setFillColor(newColor);
		}
	}
}

void Hyperlane::enablePathEffect() {
	m_pathEffectEnabled = true;
	m_pathEffectShape = m_shape;
	m_pathEffectShape.setFillColor(sf::Color::Yellow);
	m_pathEffectClock.restart();
}