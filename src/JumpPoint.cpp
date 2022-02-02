#include "gamepch.h"

#include "JumpPoint.h"
#include "TextureCache.h"
#include "Hyperlane.h"
#include "Star.h"
#include "Debug.h"
#include "Random.h"
#include "Math.h"
#include "Renderer.h"

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
	m_hyperlaneID = hyperlane->getID();
	m_isOutgoing = isOutgoing;
}

void JumpPoint::draw(Renderer& renderer, const sf::RenderWindow& window) {
	m_sprite.rotate(60.0f / (1.0f / m_rotationClock.getElapsedTime().asSeconds()));
	m_rotationClock.restart();

	if (isMouseInRadius(window, renderer)) {
		renderer.effects.drawGlow(renderer, m_sprite.getPosition(), getRadius() * 10.0f, sf::Color(255, 0, 255));
	}

	renderer.effects.drawWithDistanceShader(renderer, m_trail, renderer.mapCoordsToPixel(m_sprite.getPosition()));
	renderer.draw(m_sprite);
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
	currentStar->addEffectAnimation(EffectAnimation(EffectAnimation::Effect::JUMP, ship->getPos(), 60));

	JumpPoint* nextPoint = getConnectedJumpPoint();
	ship->setPos(nextPoint->getPos());
	
	Star* nextStar = getConnectedOtherStar();
	currentStar->moveShipToOtherStar(ship, nextStar);

	sf::Vector2f randVel = Random::randVec(-50.0f, 50.0f);
	ship->addVelocity(randVel);


	m_hyperlane->addJumpEffect(ship->getFactionColor(), currentStar);
	m_hyperlane->onJump();
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

bool JumpPoint::isMouseInRadius(const sf::RenderWindow& window, const Renderer& renderer) {
	sf::Vector2f mouseWorldPos = renderer.mapPixelToCoords(sf::Mouse::getPosition(window));
	if (Math::distance(mouseWorldPos, m_sprite.getPosition()) < getRadius()) {
		return true;
	}
	return false;
}

void JumpPoint::reinitAfterLoad(Star* star) {
	m_hyperlane = star->getHyperlaneByID(m_hyperlaneID);
}

void JumpPoint::onClick(GameState& state, sf::Vector2f releaseMouseWorldPos, sf::Vector2f pressMouseWorldPos) {
	if (isPointInRadius(releaseMouseWorldPos) && isPointInRadius(pressMouseWorldPos)) {
		state.changeToWorldView();
		state.changeToLocalView(getConnectedOtherStar());

		state.getCamera().setPos(getConnectedJumpPoint()->getPos());
	}
}
