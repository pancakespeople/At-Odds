#include "gamepch.h"
#include "Asteroid.h"
#include "TextureCache.h"
#include "Renderer.h"
#include "SmoothCircle.h"
#include "Faction.h"
#include "Random.h"

const std::vector<std::string> textures = {
	"data/art/asteroid1.png",
	"data/art/asteroid2.png",
	"data/art/asteroid3.png",
	"data/art/asteroid4.png",
	"data/art/asteroid5.png",
	"data/art/asteroid6.png",
	"data/art/asteroid7.png",
	"data/art/asteroid8.png"
};

Asteroid::Asteroid(sf::Vector2f pos, sf::Vector2f starPos) {
	m_sprite.setTexture(TextureCache::getTexture(textures[Random::randInt(0, textures.size() - 1)]));
	m_sprite.setOrigin({ m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f });
	m_sprite.setPosition(pos);
	m_sprite.setScale({ 4.0f, 4.0f });

	m_orbit = Orbit(pos, starPos);
}

void Asteroid::draw(Renderer& renderer) {
	float radius = m_sprite.getLocalBounds().width * 1.25f;

	SmoothCircle circle;
	circle.setOrigin({ radius, radius });
	circle.setPosition(m_sprite.getPosition());
	circle.setColor(Faction::neutralColor);
	circle.setRadius(radius);

	renderer.draw(circle);
	renderer.draw(m_sprite);
}

void Asteroid::update() {
	m_sprite.setPosition(m_orbit.update());
}