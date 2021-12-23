#include "gamepch.h"
#include "Orbit.h"
#include "Math.h"

Orbit::Orbit(sf::Vector2f orbiterPos, sf::Vector2f parentPos, float orbitSpeed) {
	m_radius = Math::distance(orbiterPos, parentPos);
	m_parentPos = parentPos;
	
	m_orbiterAngle = Math::angleBetween(orbiterPos, parentPos);
	m_orbitSpeed = orbitSpeed;

	m_orbitIndicator.setFillColor(sf::Color::Transparent);
	m_orbitIndicator.setOutlineColor(sf::Color(175, 175, 175, 150));
	m_orbitIndicator.setOutlineThickness(10.0f);
	m_orbitIndicator.setRadius(m_radius);
	m_orbitIndicator.setOrigin(sf::Vector2f(m_radius, m_radius));
	m_orbitIndicator.setPosition(parentPos);
	m_orbitIndicator.setPointCount(100);
}

sf::Vector2f Orbit::update() {
	sf::Vector2f newPos(m_radius * std::cos(m_orbiterAngle * Math::toRadians) + m_parentPos.x, 
						m_radius * std::sin(m_orbiterAngle * Math::toRadians) + m_parentPos.y);
	m_orbiterAngle += m_orbitSpeed;

	return newPos;
}

void Orbit::draw(sf::RenderWindow& window) {
	m_orbitIndicator.setPosition(m_parentPos);
	window.draw(m_orbitIndicator);
}