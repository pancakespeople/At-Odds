#include "gamepch.h"

#include "Projectile.h"
#include "Math.h"
#include "Debug.h"
#include "TextureCache.h"
#include "TOMLCache.h"
#include "Star.h"
#include "Sounds.h"
#include "Random.h"
#include "Animation.h"
#include "Renderer.h"
#include "Script.h"

Projectile::Projectile(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/projectiles.toml");

	assert(table.contains(type));

	m_damage = table[type]["damage"].value_or(10.0f);
	m_life = table[type]["life"].value_or(100.0f);
	m_speed = table[type]["speed"].value_or(10.0f);

	std::string texturePath = table[type]["texturePath"].value_or("");

	if (texturePath != "") {
		m_sprite.setTexture(TextureCache::getTexture(table[type]["texturePath"].value_or("")));
	}
	m_sprite.setScale(table[type]["scaleX"].value_or(1.0f), table[type]["scaleY"].value_or(1.0f));

	m_onDeathScript = table[type]["onDeathScript"].value_or("");
	m_diesOnCollision = table[type]["diesOnCollision"].value_or(true);
	m_collider.setRadius(table[type]["radius"].value_or(1.0f));
	m_orbitalBombardment = table[type]["orbitalBombardment"].value_or(false);
	m_planetaryInvasion = table[type]["planetaryInvasion"].value_or(false);

	init(sf::Vector2f(0.0f, 0.0f), 90.0f, -1);
}

Projectile::Projectile() {
	init(sf::Vector2f(0.0f, 0.0f), 90.0f, -1);
}

void Projectile::init(const sf::Vector2f& pos, float angleDegrees, int allegiance) {
	m_angle = angleDegrees;
	
	m_sprite.setOrigin(sf::Vector2f(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f));
	m_sprite.setPosition(pos);
	m_sprite.rotate(-angleDegrees);

	m_allegiance = allegiance;
	m_collider.setPosition(pos);
}

void Projectile::update(Star* star) {
	m_sprite.move(sf::Vector2f(std::cos(m_angle * Math::toRadians) * m_speed, -std::sin(m_angle * Math::toRadians) * m_speed));
	m_collider.update(m_sprite.getPosition());
	m_life -= 1.0f;
}

void Projectile::draw(Renderer& renderer) {
	renderer.draw(m_sprite);
}

bool Projectile::isCollidingWith(const Collider& collider) {
	float dist = Math::distance(m_collider.getPosition(), collider.getPosition());
	if (dist - m_collider.getRadius() <= collider.getRadius()) {
		return true;
	}
	else {
		return false;
	}
}

void Projectile::setPos(const sf::Vector2f& pos) { 
	m_sprite.setPosition(pos);
}

void Projectile::setRotation(float angleDegrees) {
	m_angle = angleDegrees; 
	m_sprite.setRotation(-angleDegrees + 90.0f);
}

void Projectile::setAllegiance(int allegiance) { 
	m_allegiance = allegiance; 
}

void Projectile::onDeath(Star* star) {
	if (m_onDeathScript != "") {
		Script::RunScript(m_onDeathScript);
		Script::CallFunction<void>("onProjectileDeath", *star, *this);
	}
}

const Collider& Projectile::getCollider() {
	return m_collider;
}
