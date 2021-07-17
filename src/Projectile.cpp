#include "gamepch.h"

#include "Projectile.h"
#include "Math.h"
#include "Debug.h"
#include "TextureCache.h"
#include "TOMLCache.h"

Projectile::Projectile(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/projectiles.toml");

	m_damage = table[type]["damage"].value_or(10.0f);
	m_life = table[type]["life"].value_or(100.0f);
	m_speed = table[type]["speed"].value_or(10.0f);

	if (table[type]["usesSprite"].value_or(false)) {
		m_sprite.setTexture(TextureCache::getTexture(table[type]["texturePath"].value_or("")));
		m_sprite.setScale(table[type]["scaleX"].value_or(1.0f), table[type]["scaleY"].value_or(1.0f));
	}
	else {
		m_shape.setScale(table[type]["scaleX"].value_or(1.0f), table[type]["scaleY"].value_or(1.0f));
		m_shape.setFillColor(sf::Color(table[type]["color"][0].value_or(255.0f), table[type]["color"][1].value_or(255.0f), table[type]["color"][2].value_or(255.0f)));
	}

	init(sf::Vector2f(0.0f, 0.0f), 90.0f, -1);
}

Projectile::Projectile() {
	init(sf::Vector2f(0.0f, 0.0f), 90.0f, -1);
}

void Projectile::init(const sf::Vector2f& pos, float angleDegrees, int allegiance) {
	m_angle = angleDegrees;
	
	if (m_usesSprite) {
		m_sprite.setPosition(pos);
		m_sprite.rotate(-angleDegrees);
	}
	else {
		m_shape.setPosition(pos);
		m_shape.setSize(sf::Vector2f(100.0f, 5.0f));
		m_shape.rotate(-angleDegrees);
	}

	m_allegiance = allegiance;
}

void Projectile::update() {
	if (m_usesSprite) {
		m_sprite.move(sf::Vector2f(std::cos(m_angle * Math::toRadians) * m_speed, -std::sin(m_angle * Math::toRadians) * m_speed));
	}
	else {
		m_shape.move(sf::Vector2f(std::cos(m_angle * Math::toRadians) * m_speed, -std::sin(m_angle * Math::toRadians) * m_speed));
	}
	m_life -= 1.0f;
}

void Projectile::draw(sf::RenderWindow& window) {
	if (m_usesSprite) {
		window.draw(m_sprite);
	}
	else {
		window.draw(m_shape);
	}
}

bool Projectile::isCollidingWith(const Collider& collider) {
	if (m_usesSprite) {
		float dist = Math::distance(m_sprite.getPosition(), collider.getPosition());
		if (dist <= collider.getRadius()) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		float dist = Math::distance(m_shape.getPosition(), collider.getPosition());
		if (dist <= collider.getRadius()) {
			return true;
		}
		else {
			return false;
		}
	}
}

void Projectile::setPos(const sf::Vector2f& pos) { 
	if (m_usesSprite) {
		m_sprite.setPosition(pos);
	}
	else {
		m_shape.setPosition(pos);
	}
}

void Projectile::setRotation(float angleDegrees) {
	m_angle = angleDegrees; 
	
	if (m_usesSprite) {
		m_sprite.setRotation(-angleDegrees + 90.0f);
	}
	else {
		m_shape.setRotation(-angleDegrees);
	}
}

void Projectile::setAllegiance(int allegiance) { 
	m_allegiance = allegiance; 
}