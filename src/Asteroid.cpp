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
	float rnd = Random::randFloat(0.0f, 1.0f);

	if (rnd > 0.5f) {
		// 50% chance
		m_resource = "COMMON_ORE";
		m_sprite.setColor(sf::Color::Red);
	}
	else if (rnd > 0.15f) {
		// 35% chance
		m_resource = "UNCOMMON_ORE";
		m_sprite.setColor(sf::Color(0, 255, 255));
	}
	else {
		// 15% chance
		m_resource = "RARE_ORE";
		m_sprite.setColor(sf::Color::Yellow);
	}

	//m_resourceCount = Random::randFloat(500.0f, 5000.0f);
	m_resourceCount = 100.0f;
}

void Asteroid::draw(Renderer& renderer) {
	float radius = getRadius();

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

	if (m_resourceCount <= 0.0f) {
		m_destructionTimer++;
	}
}

void Asteroid::mineAsteroid(Faction& faction, float amount) {
	if (m_resourceCount - amount > 0.0f) {
		faction.addResource(m_resource, amount);
		m_resourceCount -= amount;
	}
	else if (m_resourceCount > 0.0f) {
		faction.addResource(m_resource, m_resourceCount);
		m_resourceCount = 0.0f;
	}
}