#include "gamepch.h"

#include "Projectile.h"
#include "Math.h"
#include "Debug.h"
#include "TextureCache.h"

Projectile::Projectile(PROJECTILE_TYPE type) {
	switch (type) {
	case PROJECTILE_TYPE::LASER:
		m_shape.setFillColor(sf::Color(255.0f, 0.0f, 0.0f));

		m_damage = 10.0f;
		m_life = 100.0f;
		m_speed = 10.0f;
		break;
	case PROJECTILE_TYPE::GAUSS:
		m_usesSprite = true;
		m_sprite.setTexture(TextureCache::getTexture("data/art/gaussproj.png"));
		m_sprite.setScale(sf::Vector2f(2.0f, 2.0f));

		m_life = 500.0f;
		m_damage = 25.0f;
		m_speed = 10.0f;
		break;
	case PROJECTILE_TYPE::LIGHT_BALLISTIC:
		m_shape.setFillColor(sf::Color::Yellow);

		m_damage = 1.0f;
		m_life = 100.0f;
		m_speed = 15.0f;
		break;
	case PROJECTILE_TYPE::LONG_RANGE_LASER:
		m_shape.setFillColor(sf::Color(255.0f, 0.0f, 0.0f));

		m_damage = 10.0f;
		m_life = 1000.0f;
		m_speed = 10.0f;
		break;
	case PROJECTILE_TYPE::LONG_RANGE_LIGHT_BALLISTIC:
		m_shape.setFillColor(sf::Color::Yellow);

		m_damage = 1.0f;
		m_life = 1000.0f;
		m_speed = 15.0f;
		break;
	case PROJECTILE_TYPE::CONSTRUCTION:
		m_shape.setFillColor(sf::Color(100, 100, 255));
		m_shape.setScale(sf::Vector2f(1.0f, 4.0f));

		m_damage = 0.0f;
		m_life = 100.0f;
		m_speed = 15.0f;
		break;
	default:
		DEBUG_PRINT("Invalid projectile type");
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