#include "gamepch.h"

#include "JumpPoint.h"
#include "TextureCache.h"
#include "Hyperlane.h"
#include "Star.h"
#include "Debug.h"

JumpPoint::JumpPoint(sf::Vector2f pos, float angleRadians, Hyperlane* hyperlane, bool isOutgoing) {
	m_sprite.setTexture(TextureCache::getTexture("data/art/swirly2.png"));
	m_sprite.setOrigin(m_sprite.getTextureRect().width / 2.0f, m_sprite.getTextureRect().height / 2.0f);
	m_sprite.setPosition(pos);
	
	// Nicer position
	sf::Vector2f trailPos = pos;
	trailPos.x += std::sin(angleRadians) * (250.0f / 2.0f);
	trailPos.y -= std::cos(angleRadians) * (250.0f / 2.0f);

	m_trail.setPosition(trailPos);
	m_trail.setSize(sf::Vector2f(10000.0f, 250.0f));
	m_trail.setFillColor(sf::Color(255.0f, 0.0f, 255.0f, 125.0f));
	m_trail.rotate(angleRadians * (180 / 3.14159));

	m_hyperlane = hyperlane;
	m_isOutgoing = isOutgoing;
}

void JumpPoint::draw(sf::RenderWindow& window, EffectsEmitter& emitter) {
	m_sprite.rotate(60.0f / (1.0f / m_rotationClock.getElapsedTime().asSeconds()));
	m_rotationClock.restart();

	emitter.drawWithDistanceShader(window, m_trail, window.mapCoordsToPixel(m_sprite.getPosition()));
	window.draw(m_sprite);
}

JumpPoint* JumpPoint::getConnectedJumpPoint() {
	Star* nextStar;
	
	if (isOutgoing()) {
		nextStar = m_hyperlane->getEndStar();
	}
	else {
		nextStar = m_hyperlane->getBeginStar();
	}

	for (JumpPoint& h : nextStar->getJumpPoints()) {
		if (h.getHyperlane() == m_hyperlane) {
			return &h;
		}
	}
	DEBUG_PRINT("Error: A star has no jump points");
	return nullptr;
}

Star* JumpPoint::getConnectedOtherStar() {
	Star* nextStar;

	if (isOutgoing()) {
		nextStar = m_hyperlane->getEndStar();
	}
	else {
		nextStar = m_hyperlane->getBeginStar();
	}

	return nextStar;
}

void JumpPoint::jumpShipThrough(Spaceship* ship, Star* currentStar) {
	JumpPoint* nextPoint = getConnectedJumpPoint();
	ship->setPos(nextPoint->getPos());
	
	Star* nextStar = getConnectedOtherStar();
	currentStar->removeSpaceship(ship);
	nextStar->addSpaceship(ship);
}

bool JumpPoint::isPointInRadius(sf::Vector2f point) {
	float radius = m_sprite.getTextureRect().width / 2.0f;
	sf::Vector2f thisPos = getPos();

	float distX = point.x - thisPos.x;
	float distY = point.y - thisPos.y;

	if (sqrt(std::pow(distX, 2) + std::pow(distY, 2)) <= radius) {
		return true;
	}
	else return false;
}
