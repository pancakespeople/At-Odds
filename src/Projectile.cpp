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

const std::map<std::string, std::function<void(Star* star, Projectile* proj)>> deathFunctions = {
	{"smallExplosion", &DeathFunctions::smallExplosion},
	{"largeExplosion", &DeathFunctions::largeExplosion},
	{"laserRing", &DeathFunctions::laserRing},
	{"lightningAOE", &DeathFunctions::lightningAOE},
	{"knockAll", &DeathFunctions::knockAll},
};

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

	m_deathFunctionName = table[type]["deathFunction"].value_or("");
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

void Projectile::draw(sf::RenderWindow& window) {
	window.draw(m_sprite);
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
	// Call death function
	if (deathFunctions.count(m_deathFunctionName) > 0) deathFunctions.at(m_deathFunctionName)(star, this);
}

const Collider& Projectile::getCollider() {
	return m_collider;
}

void DeathFunctions::smallExplosion(Star* star, Projectile* proj) {
	star->addAnimation(Animation("SMALL_EXPLOSION", proj->getPos()));
	Sounds::playSoundLocal("data/sound/boom2.wav", star, proj->getPos(), 25, 1.0f + Random::randFloat(-0.5f, 0.5f));
}

void DeathFunctions::largeExplosion(Star* star, Projectile* proj) {
	star->addAnimation(Animation("EXPLOSION", proj->getPos()));
	Sounds::playSoundLocal("data/sound/boom1.wav", star, proj->getPos(), 25, 1.0f + Random::randFloat(-0.5f, 0.5f));
}

void DeathFunctions::laserRing(Star* star, Projectile* proj) {
	float angle = 0.0f;
	for (int i = 0; i < 8; i++) {
		Projectile p("LASER");
		p.setPos(proj->getPos());
		p.setRotation(angle);
		p.setAllegiance(proj->getAllegiance());
		angle += 45.0f;
		star->addProjectile(p);
	}
}

void DeathFunctions::lightningAOE(Star* star, Projectile* proj) {
	star->addAnimation(Animation("LIGHTNING", proj->getPos()));
	Projectile p("LIGHTNING_AOE");
	p.setPos(proj->getPos());
	p.setAllegiance(proj->getAllegiance());
	star->addProjectile(p);
}

void DeathFunctions::knockAll(Star* star, Projectile* proj) {
	star->addAnimation(Animation("EXPLOSION", proj->getPos()));
	Sounds::playSoundLocal("data/sound/boom1.wav", star, proj->getPos(), 50.0f, 0.25f);

	auto& spaceships = star->getSpaceships();
	for (auto& ship : spaceships) {
		float dist = Math::distance(proj->getPos(), ship->getPos());
		float accel = 1000000000.0f / ship->getMass() / (dist * 4.0f);
		float angle = Math::angleBetween(proj->getPos(), ship->getPos()) + 180.0f;
		ship->addVelocity(sf::Vector2f(std::cos(angle * Math::toRadians), std::sin(angle * Math::toRadians) * accel));
	}
}