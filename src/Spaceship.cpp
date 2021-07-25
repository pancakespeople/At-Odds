#include "gamepch.h"

#include "Spaceship.h"
#include "TextureCache.h"
#include "Star.h"
#include "Debug.h"
#include "Math.h"
#include "JumpPoint.h"
#include "Hyperlane.h"
#include "EffectsEmitter.h"
#include "Sounds.h"
#include "Faction.h"
#include "Random.h"
#include "Animation.h"
#include "Building.h"
#include "Mod.h"
#include "toml.hpp"
#include "TOMLCache.h"

void Spaceship::init(const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color) {
	m_sprite.setPosition(pos);
	m_sprite.setOrigin(sf::Vector2f(m_sprite.getTextureRect().width / 2.0f, m_sprite.getTextureRect().height / 2.0f));

	m_maxAcceleration = 25000.0f / m_mass;
	m_rotationSpeed = 50000.0f / m_mass;

	m_currentStar = star;

	m_collider.setPosition(pos);
	m_collider.setOutlineColor(color);

	m_allegiance = allegiance;

	enableAllMods();
}

Spaceship::Spaceship(const std::string& type, const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color) {
	const toml::table& table = TOMLCache::getTable("data/objects/spaceships.toml");
	
	assert(table.contains(type));

	std::string texturePath = table[type]["texturePath"].value_or("");
	
	float scale = table[type]["scale"].value_or(1.0f);
	float mass = table[type]["mass"].value_or(50000.0f);
	float health = table[type]["health"].value_or(100.0f);
	float colliderRadius = table[type]["colliderRadius"].value_or(100.0f);
	float constructionSpeed = table[type]["constructionSpeed"].value_or(0.0f);
	
	bool spriteTakesFactionColor = table[type]["spriteTakesFactionColor"].value_or(false);
	bool canReceiveOrders = table[type]["canReceiveOrders"].value_or(true);
	bool fighterAI = table[type]["fighterAI"].value_or(false);
	bool playerCanGiveOrders = table[type]["playerCanGiveOrders"].value_or(true);
	bool civilian = table[type]["civilian"].value_or(false);

	m_sprite.setTexture(TextureCache::getTexture(texturePath));
	m_sprite.setScale(scale, scale);
	m_mass = mass;
	m_health = health;
	m_collider.setRadius(colliderRadius);
	m_constructionSpeed = constructionSpeed;
	m_canReceiveOrders = canReceiveOrders;
	m_fighterAI = fighterAI;
	m_playerCanGiveOrders = playerCanGiveOrders;
	m_civilian = civilian;

	if (spriteTakesFactionColor) m_sprite.setColor(color);

	init(pos, star, allegiance, color);
}

void Spaceship::draw(sf::RenderWindow& window, EffectsEmitter& emitter) {
	if (m_disabled) return;
	
	window.draw(m_sprite);
	window.draw(m_collider);
	
	if (m_percentJumpDriveCharged > 0.0f) {
		emitter.drawJumpBubble(window, m_collider.getPosition(), m_collider.getRadius(), m_percentJumpDriveCharged);
	}

	if (m_selected) {
		sf::CircleShape selectionIndicator;
		
		selectionIndicator.setOrigin(sf::Vector2f(m_collider.getRadius() * 2.0f, m_collider.getRadius() * 2.0f));
		selectionIndicator.setPosition(getPos());
		selectionIndicator.setFillColor(sf::Color::Transparent);
		selectionIndicator.setOutlineColor(sf::Color::Yellow);
		selectionIndicator.setOutlineThickness(25.0f);
		selectionIndicator.setRadius(m_collider.getRadius() * 2.0f);

		window.draw(selectionIndicator);

		// Visualizes orders when selected
		if (m_orders.size() > 0) {
			m_orders.front()->draw(window, emitter, getPos());
		}
	}
}

bool Spaceship::rotateTo(float angleDegrees) {
	if (angleDegrees == m_facingAngle) return true;
	
	if (angleDegrees < 0.0f) angleDegrees += 360;

	float angleDifference = m_facingAngle - angleDegrees;
	float oppositeAngle;

	if (m_facingAngle > 180.0f) {
		oppositeAngle = m_facingAngle - 180.0f;
		// Turn clockwise
		if (angleDegrees > oppositeAngle && angleDegrees < m_facingAngle) {
			m_sprite.rotate(m_rotationSpeed);
			m_facingAngle -= m_rotationSpeed;
		}
		// Turn counterclockwise
		else {
			m_sprite.rotate(-m_rotationSpeed);
			m_facingAngle += m_rotationSpeed;
		}
	}
	else {
		oppositeAngle = m_facingAngle + 180.0f;
		// Turn counterclockwise
		if (angleDegrees < oppositeAngle && angleDegrees > m_facingAngle) {
			m_sprite.rotate(-m_rotationSpeed);
			m_facingAngle += m_rotationSpeed;
		}
		// Turn clockwise
		else {
			m_sprite.rotate(m_rotationSpeed);
			m_facingAngle -= m_rotationSpeed;
		}
	}

	if (m_facingAngle < 0.0f) {
		m_facingAngle = 360.0f + m_facingAngle;
	}

	if (m_facingAngle >= 360.0f) {
		m_facingAngle = 0.0f + (360.0f - m_facingAngle);
	}

	return false;
}

void Spaceship::accelerate(float amount) {
	float speed = std::sqrt(std::pow(m_velocity.x, 2) + std::pow(m_velocity.y, 2));
	/*if (speed > m_maxSpeed && amount > 0.0f) {
		m_velocity.x = std::sqrt(std::pow(m_maxSpeed, 2) - std::pow(m_velocity.y, 2)) * std::cos(m_facingAngle * Math::toRadians);
		m_velocity.y = std::sqrt(std::pow(m_maxSpeed, 2) - std::pow(m_velocity.x, 2)) * -std::sin(m_facingAngle * Math::toRadians);
		return;
	}*/

	m_velocity.x += std::cos(m_facingAngle * Math::toRadians) * amount;
	m_velocity.y += -std::sin(m_facingAngle * Math::toRadians) * amount;
}

void Spaceship::update(Star* currentStar) {
	if (m_disabled) return;
	
	m_currentStar = currentStar;
	
	if (!m_dead && m_health <= 0.0f) {
		m_dead = true;
		if (!m_diesSilently) {
			m_currentStar->addAnimation(Animation("EXPLOSION", getPos()));
			Sounds::playSoundLocal("data/sound/boom1.wav", m_currentStar, getPos(), 25, 1.0f + Random::randFloat(-0.5f, 0.5f));
		}
	}

	if (!m_orders.empty()) {
		if (m_orders.front()->execute(this, currentStar)) {
			m_orders.pop_front();
		}
	}
	else if (m_weapons.size() > 0) {
		// Attack enemies in system
		
		std::vector<Spaceship*> enemies = findEnemyCombatShips();
		if (enemies.size() > 0) {
			attackRandomEnemy(enemies);
		}
		else {
			std::vector<Building*> enemyBuildings = findEnemyBuildings();
			if (enemyBuildings.size() > 0) {
				attackRandomEnemyBuilding(enemyBuildings);
			}
		}
	}
	
	m_velocity.x *= 0.99f;
	m_velocity.y *= 0.99f;
	m_sprite.move(m_velocity);

	//m_weapon.fireAt(this, m_currentStar->getLocalViewCenter(), m_currentStar);
	for (Weapon& w : m_weapons) {
		w.update();
	}

	m_collider.update(getPos());
}

float Spaceship::angleTo(const sf::Vector2f& pos) {
	sf::Vector2f thisPos = m_sprite.getPosition();
	sf::Vector2f dist;

	dist.x = pos.x - thisPos.x;
	dist.y = pos.y - thisPos.y;

	float angleDegrees = std::atan(-dist.y / dist.x) * Math::toDegrees;
	angleDegrees += 180.0f;
	
	if (thisPos.x < pos.x) {
		angleDegrees = angleDegrees - 180.0f;
	}

	if (angleDegrees < 0.0f) {
		angleDegrees += 360.0f;
	}

	/*if (angleDegrees > 360.0f) {
		angleDegrees -= 360.0f;
	}
	
	if (angleDegrees < 360.0f) {
		angleDegrees += 360.0f;
	}*/

	return angleDegrees;
}

bool Spaceship::flyTo(const sf::Vector2f& pos) {
	float angle = angleTo(pos);
	float dist = Math::distance(m_sprite.getPosition(), pos) - m_collider.getRadius();
	float closeArea = 25000000 / m_mass;
	float farArea = 250000000 / m_mass;
	
	if (dist > closeArea) {
		rotateTo(angle);
		if (dist > farArea) {
			accelerate(m_maxAcceleration);
		}
		else if (dist > closeArea) {
			keepSpeed(10.0f);
		}
	}
	else if (getSpeed() < 5.0f) {
		return true;
	}
	else {
		return true;
	}
	return false;
}

float Spaceship::getSpeed() {
	return std::sqrt(std::pow(m_velocity.x, 2) + std::pow(m_velocity.y, 2));
}

void Spaceship::keepSpeed(float speed) {
	float nowSpeed = getSpeed();
	static float lastSpeed = getSpeed();
	float changeSpeed = lastSpeed - nowSpeed;

	if (nowSpeed > speed) {
		accelerate(-(m_maxAcceleration / 5.0f));
	}
	else if (nowSpeed < speed) {
		accelerate(m_maxAcceleration / 5.0f);
	}

	lastSpeed = getSpeed();
}

Spaceship::JumpState Spaceship::jump(JumpPoint* point) {
	if (!flyTo(point->getPos())) {
		return Spaceship::JumpState::TRAVEL;
	}
	else {
		if (m_percentJumpDriveCharged >= 100.0f) {
			point->jumpShipThrough(this, m_currentStar);
			m_percentJumpDriveCharged = 0.0f;
			Sounds::playSoundLocal("data/sound/woosh1.wav", m_currentStar, getPos(), 100, 1.0f + Random::randFloat(-0.5f, 0.5f), true);
			Sounds::playSoundLocal("data/sound/woosh4.wav", point->getConnectedOtherStar(), getPos(), 100, 1.0f + Random::randFloat(-0.5f, 0.5f), true);
			m_currentStar = point->getConnectedOtherStar();
			return Spaceship::JumpState::DONE;
		}
		else {
			m_percentJumpDriveCharged += 1.0f;
			return Spaceship::JumpState::JUMPING;
		}
	}
}

std::vector<Spaceship*> Spaceship::findEnemyShips() {
	std::vector<std::unique_ptr<Spaceship>>& allShips = m_currentStar->getSpaceships();
	std::vector<Spaceship*> enemies;
	for (auto& s : allShips) {
		if (s->getAllegiance() != m_allegiance) {
			enemies.push_back(s.get());
		}
	}
	return enemies;
}

std::vector<Spaceship*> Spaceship::findEnemyCombatShips() {
	std::vector<std::unique_ptr<Spaceship>>& allShips = m_currentStar->getSpaceships();
	std::vector<Spaceship*> enemies;
	for (auto& s : allShips) {
		if (s->getAllegiance() != m_allegiance && !s->isCivilian()) {
			enemies.push_back(s.get());
		}
	}
	return enemies;
}

void Spaceship::fireAt(Spaceship* target, int weaponIdx) {
	m_weapons[weaponIdx].fireAt(this, target->getPos(), m_currentStar);
}

void Spaceship::smartFireAt(Unit* target, int weaponIdx) {
	sf::Vector2f targetPos = target->getPos();
	sf::Vector2f targetVel = target->getVelocity();
	float dist = Math::distance(getPos(), targetPos);
	
	float hyp = std::sqrt(std::pow(targetVel.x, 2) + std::pow(targetVel.y, 2));
	float projectileSpeed = 10.0f;
	
	sf::Vector2f predictPos(targetPos.x + targetVel.x * (projectileSpeed * hyp), targetPos.y + targetVel.y * (projectileSpeed * hyp));

	m_weapons[weaponIdx].fireAt(this, predictPos, m_currentStar);
}

void Spaceship::orbit(const sf::Vector2f& pos) {
	float angle = angleTo(pos);
	float dist = Math::distance(m_sprite.getPosition(), pos);
	float closeArea = 25000000 / m_mass;
	float farArea = 250000000 / m_mass;

	rotateTo(angle);
	
	if (dist > farArea) {
		accelerate(m_maxAcceleration);
	}
	else {
		accelerate(m_maxAcceleration / 2.0);
	}
}

void Spaceship::captureCurrentStar(Faction* faction) {
	if (m_currentStar->getAllegiance() != m_allegiance) {
		if (findEnemyCombatShips().size() == 0) {
			m_currentStar->factionTakeOwnership(faction);
			m_currentStar->createSpaceship(std::make_unique<Spaceship>("CLAIM_SHIP", getPos(), m_currentStar, m_allegiance, m_collider.getOutlineColor()));
		}
	}
}

void Spaceship::attackRandomEnemy(std::vector<Spaceship*>& enemies, bool urgent) {
	int randIndex;
	if (enemies.size() > 1) {
		randIndex = Random::randInt(0, enemies.size() - 1);
	}
	else {
		randIndex = 0;
	}
	if (enemies.size() > 0) {
		if (m_fighterAI) {
			if (urgent) {
				addOrderFront(AttackOrder(enemies[randIndex], true));
			}
			else {
				addOrder(AttackOrder(enemies[randIndex], true));
			}
		}
		else if (urgent) {
			addOrderFront(AttackOrder(enemies[randIndex]));
		}
		else {
			addOrder(AttackOrder(enemies[randIndex]));
		}
	}
}

void Spaceship::onSelected() {
	m_selected = true;
}

void Spaceship::onDeselected() {
	m_selected = false;
}

std::vector<Building*> Spaceship::findEnemyBuildings() {
	std::vector<Building*> buildings;
	for (auto& b : m_currentStar->getBuildings()) {
		if (b->getAllegiance() != m_allegiance) {
			buildings.push_back(b.get());
		}
	}
	return buildings;
}

void Spaceship::attackRandomEnemyBuilding(std::vector<Building*>& enemyBuildings) {
	int randIndex;
	if (enemyBuildings.size() > 1) {
		randIndex = Random::randInt(0, enemyBuildings.size() - 1);
	}
	else {
		randIndex = 0;
	}
	if (enemyBuildings.size() > 0) {
		addOrder(AttackOrder(enemyBuildings[randIndex]));
	}
}

std::unordered_map<PlanetResource::RESOURCE_TYPE, float> Spaceship::DesignerShip::getTotalResourceCost() {
	std::unordered_map<PlanetResource::RESOURCE_TYPE, float> totalResourceCost;
	
	for (auto& resource : chassis.resourceCost) {
		totalResourceCost[resource.first] += resource.second;
	}

	for (DesignerWeapon& weapon : weapons) {
		for (auto& resource : weapon.resourceCost) {
			totalResourceCost[resource.first] += resource.second;
		}
	}

	return totalResourceCost;
}

Spaceship::DesignerChassis::DesignerChassis(const std::string& typeStr) {
	const toml::table& table = TOMLCache::getTable("data/objects/chassis.toml");

	assert(table.contains(typeStr));

	type = table[typeStr]["type"].value_or("");
	name = table[typeStr]["name"].value_or("");
	maxWeaponCapacity = table[typeStr]["maxWeaponCapacity"].value_or(1.0f);

	for (int i = 0; i < table[typeStr]["cost"].as_array()->size(); i++) {
		PlanetResource::RESOURCE_TYPE resourceType = static_cast<PlanetResource::RESOURCE_TYPE>(table[typeStr]["cost"][i][0].value_or(0));
		resourceCost[resourceType] = table[typeStr]["cost"][i][1].value_or(0.0f);
	}
}

Spaceship::DesignerWeapon::DesignerWeapon(const std::string& typeStr) {
	const toml::table& table = TOMLCache::getTable("data/objects/weapondesigns.toml");

	assert(table.contains(typeStr));

	type = table[typeStr]["type"].value_or("");
	name = table[typeStr]["name"].value_or("");
	weaponPoints = table[typeStr]["weaponPoints"].value_or(1.0f);

	for (int i = 0; i < table[typeStr]["cost"].as_array()->size(); i++) {
		PlanetResource::RESOURCE_TYPE resourceType = static_cast<PlanetResource::RESOURCE_TYPE>(table[typeStr]["cost"][i][0].value_or(0));
		resourceCost[resourceType] = table[typeStr]["cost"][i][1].value_or(0.0f);
	}
}

std::string Spaceship::DesignerShip::generateName() {
	std::string weaponName;
	if (weapons.size() > 0) {
		weaponName = weapons[0].name;
	}
	return weaponName + " " + chassis.name;
}

float Spaceship::DesignerShip::getTotalWeaponPoints() {
	float total = 0.0f;
	for (auto& weapon : weapons) {
		total += weapon.weaponPoints;
	}
	return total;
}