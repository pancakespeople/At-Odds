#include "gamepch.h"
#include "Building.h"
#include "TextureCache.h"
#include "Random.h"
#include "Animation.h"
#include "Star.h"
#include "Sounds.h"

Building::Building(BUILDING_TYPE type, Star* star, sf::Vector2f pos, int allegiance, sf::Color color) {
	switch (type) {
	case BUILDING_TYPE::OUTPOST:
		m_sprite.setTexture(TextureCache::getTexture("data/art/outpost.png"));
		m_sprite.setScale(sf::Vector2f(2.0f, 2.0f));
		break;
	}

	m_collider = Collider(pos, color, m_sprite.getLocalBounds().width * m_sprite.getScale().x / 1.5f);
	
	m_sprite.setPosition(pos);
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
	m_sprite.setRotation(Random::randFloat(0.0f, 360.0f));
	
	m_allegiance = allegiance;

	m_currentStar = star;
}

void Building::draw(sf::RenderWindow& window) {
	window.draw(m_sprite);
	window.draw(m_collider);
}

void Building::update(Star* star) {
	if (!m_dead && m_health <= 0.0f) {
		m_dead = true;
		star->addAnimation(Animation(Animation::ANIMATION_TYPE::EXPLOSION, getPos()));
		Sounds::playSoundLocal("data/sound/boom1.wav", star, 25, 1.0f + Random::randFloat(-0.5f, 0.5f));
	}
	
	m_sprite.rotate(0.1f);
}