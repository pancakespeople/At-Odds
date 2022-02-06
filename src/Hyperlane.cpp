#include "gamepch.h"

#include "Hyperlane.h"
#include "Debug.h"
#include "Constellation.h"
#include "Renderer.h"
#include "Math.h"

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
	endpos.x += std::sin(m_angleDegrees * (3.14159 / 180)) * (HYPERLANE_THICKNESS / 2.0f);
	endpos.y -= std::cos(m_angleDegrees * (3.14159 / 180)) * (HYPERLANE_THICKNESS / 2.0f);
	
	m_shape.setPosition(hyPos);

	sf::FloatRect bounds = m_shape.getLocalBounds();
	
	m_vertices.push_back(sf::Vertex(hyPos));
	m_vertices.push_back(sf::Vertex(hyPos + sf::Vector2f(std::cos((m_angleDegrees + 90.0f) * (3.14159 / 180.0f)) * HYPERLANE_THICKNESS, 
														 std::sin((m_angleDegrees + 90.0f) * (3.14159 / 180.0f)) * HYPERLANE_THICKNESS)));
	m_vertices.push_back(sf::Vertex(endpos + sf::Vector2f(std::cos((m_angleDegrees + 90.0f) * (3.14159 / 180.0f)) * HYPERLANE_THICKNESS,
														  std::sin((m_angleDegrees + 90.0f) * (3.14159 / 180.0f)) * HYPERLANE_THICKNESS)));
	m_vertices.push_back(sf::Vertex(endpos));

	begin->connectHyperlane(this);
	end->connectHyperlane(this);

	m_beginStarID = begin->getID();
	m_endStarID = end->getID();
}

void Hyperlane::draw(Renderer& renderer, int playerFaction) {
	// Set faction colors
	if (m_beginStar->isDiscovered(playerFaction)) {
		if (m_beginStar->getAllegiance() != -1) {
			m_vertices[0].color = m_beginStar->getColor();
			m_vertices[1].color = m_beginStar->getColor();
		}
		else {
			m_vertices[0].color = sf::Color(255, 255, 255, 100);
			m_vertices[1].color = sf::Color(255, 255, 255, 100);
		}
	}
	else {
		m_vertices[0].color = sf::Color(255, 255, 255, 100);
		m_vertices[1].color = sf::Color(255, 255, 255, 100);
	}

	if (m_endStar->isDiscovered(playerFaction)) {
		if (m_endStar->getAllegiance() != -1) {
			m_vertices[2].color = m_endStar->getColor();
			m_vertices[3].color = m_endStar->getColor();
		}
		else {
			m_vertices[2].color = sf::Color(255, 255, 255, 100);
			m_vertices[3].color = sf::Color(255, 255, 255, 100);
		}
	}
	else {
		m_vertices[2].color = sf::Color(255, 255, 255, 100);
		m_vertices[3].color = sf::Color(255, 255, 255, 100);
	}
	
	if (m_beginStar->isDiscovered(playerFaction) && m_endStar->isDiscovered(playerFaction)) {
		if (m_jumpEffects.size() > 0) {
			if (m_jumpEffects.front().clock.getElapsedTime().asSeconds() > 1.0f) {
				m_jumpEffects.pop_front();
			}
		}

		for (JumpEffect& effect : m_jumpEffects) {
			float t = effect.clock.getElapsedTime().asSeconds();
			if (t < 1.0f) {
				if (effect.beginToEndJumped) {
					renderer.effects.drawGlow(Math::lerp(m_beginStar->getCenter(), m_endStar->getCenter(), t), 100.0f, effect.color);
				}
				else {
					renderer.effects.drawGlow(Math::lerp(m_endStar->getCenter(), m_beginStar->getCenter(), t), 100.0f, effect.color);
				}
			}
		}
	}
	else {
		m_jumpEffects.clear();
	}

	renderer.draw(&m_vertices[0], m_vertices.size(), sf::Quads);

	if (m_pathEffectEnabled) {
		renderer.draw(m_shape);
		
		sf::Color newColor = m_shape.getFillColor();
		
		float time = m_pathEffectClock.getElapsedTime().asSeconds() * 60.0f;
		m_pathEffectClock.restart();

		if (newColor.a - time <= 0) {
			m_pathEffectEnabled = false;
		}
		else {
			newColor.a -= time;
			m_shape.setFillColor(newColor);
		}
	}
}

void Hyperlane::enablePathEffect() {
	m_pathEffectEnabled = true;
	m_shape.setFillColor(sf::Color::Yellow);
	m_pathEffectClock.restart();
}

void Hyperlane::reinitAfterLoad(Constellation* constellation) {
	m_beginStar = constellation->getStarByID(m_beginStarID);
	m_endStar = constellation->getStarByID(m_endStarID);
}

void Hyperlane::addJumpEffect(const sf::Color& color, const Star* star) {
	if (m_jumpEffects.size() < 100) {
		bool beginToEndJumped = false;
		if (m_endStar == star) {
			beginToEndJumped = false;
		}
		else {
			beginToEndJumped = true;
		}

		m_jumpEffects.push_back({ sf::Clock(), beginToEndJumped, color });
	}
}

void Hyperlane::onJump() {
	while (m_jumpEffects.size() > 0) {
		if (m_jumpEffects.front().clock.getElapsedTime().asSeconds() > 1.0f) {
			m_jumpEffects.pop_front();
		}
		else {
			break;
		}
	}
}