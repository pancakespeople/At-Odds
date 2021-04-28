#include "gamepch.h"
#include "Building.h"
#include "TextureCache.h"
#include "Random.h"

Building::Building(BUILDING_TYPE type, sf::Vector2f pos, int allegiance, sf::Color color) {
	switch (type) {
	case BUILDING_TYPE::OUTPOST:
		m_sprite.setTexture(TextureCache::getTexture("data/art/outpost.png"));
		m_sprite.setScale(sf::Vector2f(2.0f, 2.0f));
		break;
	}

	m_collider.setRadius(m_sprite.getLocalBounds().width * m_sprite.getScale().x / 1.5f);
	m_collider.setFillColor(sf::Color::Transparent);
	m_collider.setOutlineColor(color);
	m_collider.setOutlineThickness(10.0f);
	m_collider.setOrigin(sf::Vector2f(m_collider.getRadius(), m_collider.getRadius()));
	m_collider.setPosition(pos);

	m_sprite.setPosition(pos);
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
	m_sprite.setRotation(Random::randFloat(0.0f, 360.0f));
	
	m_allegiance = allegiance;
}

void Building::draw(sf::RenderWindow& window) {
	window.draw(m_sprite);
	window.draw(m_collider);
}

void Building::update() {
	m_sprite.rotate(0.1f);
}