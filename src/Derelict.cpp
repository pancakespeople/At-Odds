#include "gamepch.h"
#include "Derelict.h"
#include "TextureCache.h"
#include "Random.h"

Derelict::Derelict(sf::Vector2f pos) {
	m_sprite.setTexture(TextureCache::getTexture("data/art/derelict.png"));
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
	m_sprite.setPosition(pos);
	m_sprite.setRotation(Random::randFloat(0.0f, 360.0f));
	m_sprite.setScale(2.0f, 2.0f);

	m_collider = Collider(pos, sf::Color(175, 175, 175), m_sprite.getLocalBounds().width * m_sprite.getScale().x / 1.5f);
}

void Derelict::draw(sf::RenderWindow& window) {
	window.draw(m_sprite);
	window.draw(m_collider);
}