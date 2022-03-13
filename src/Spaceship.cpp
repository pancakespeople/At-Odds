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
#include "ext/toml.hpp"
#include "TOMLCache.h"
#include "Constellation.h"
#include "Designs.h"
#include "Renderer.h"
#include "Script.h"

void Spaceship::init(const sf::Vector2f& pos, Star* star, int allegiance, sf::Color color) {
	m_sprite.setPosition(pos);
	m_sprite.setOrigin(sf::Vector2f(m_sprite.getTextureRect().width / 2.0f, m_sprite.getTextureRect().height / 2.0f));

	m_maxAcceleration = 25000.0f / m_mass;
	m_rotationSpeed = 50000.0f / m_mass;

	m_currentStar = star;

	m_collider.setPosition(pos);
	m_collider.setColor(color);

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
	bool stationary = table[type]["stationary"].value_or(false);

	m_sprite.setTexture(TextureCache::getTexture(texturePath));
	m_sprite.setScale(scale, scale);
	m_mass = mass;
	m_health = health;
	m_maxHealth = health;
	m_collider.setRadius(colliderRadius);
	m_constructionSpeed = constructionSpeed;
	m_canReceiveOrders = canReceiveOrders;
	m_fighterAI = fighterAI;
	m_playerCanGiveOrders = playerCanGiveOrders;
	m_civilian = civilian;
	m_stationary = stationary;
	m_type = type;

	if (spriteTakesFactionColor) m_sprite.setColor(color);

	auto* thrusterPoints = table[type]["thrusterPoints"].as_array();
	if (thrusterPoints != nullptr) {
		for (int i = 0; i < thrusterPoints->size(); i++) {
			m_thrusterPoints.push_back({ table[type]["thrusterPoints"][i][0].value_or(0.0f), table[type]["thrusterPoints"][i][1].value_or(0.0f) });
		}
	}

	m_thrusterSize = table[type]["thrusterSize"].value_or(100.0f);

	init(pos, star, allegiance, color);
}

void Spaceship::draw(Renderer& renderer, float time) {
	if (m_disabled) return;
	
	if (m_pirate) {
		sf::Sprite sprite;
		sprite.setTexture(TextureCache::getTexture("data/art/skullncrossbones.png"));
		sprite.setOrigin(sprite.getLocalBounds().width / 2.0f, sprite.getLocalBounds().height);
		sprite.setScale(sf::Vector2f(m_collider.getRadius() / 100.0f, m_collider.getRadius() / 100.0f));
		sprite.setPosition(m_sprite.getPosition() - sf::Vector2f(0.0f, m_collider.getRadius()));
		sprite.setColor(sf::Color(255, 255, 255, (std::sin(time * 4.0f) + 1.0f) / 2.0f * 255));
		renderer.draw(sprite);
	}

	renderer.draw(m_sprite);
	renderer.draw(m_collider);
	
	if (m_percentJumpDriveCharged > 0.0f) {
		renderer.effects.drawJumpBubble(m_collider.getPosition(), m_collider.getRadius(), m_percentJumpDriveCharged);
	}

	if (m_selected) {
		drawSelectionCircle(renderer);

		// Visualizes orders when selected
		for (int i = 0; i < m_orders.size(); i++) {
			if (i == 0) {
				m_orders.front()->draw(renderer, getPos(), m_currentStar);
			}
			else {
				int j = i - 1;
				std::pair<bool, sf::Vector2f> destPos = m_orders[j]->getDestinationPos(m_currentStar);

				while (!destPos.first && j != 0) {
					j--;
					destPos = m_orders[j]->getDestinationPos(m_currentStar);
				}

				if (destPos.first) {
					m_orders[i]->draw(renderer, destPos.second, m_currentStar);
				}
			}
		}

		drawHealthBar(renderer);
	}

	for (Weapon& weapon : m_weapons) {
		weapon.drawFireAnimation(renderer, this);
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
	
	m_velocity.x += std::cos(m_facingAngle * Math::toRadians) * amount;
	m_velocity.y += -std::sin(m_facingAngle * Math::toRadians) * amount;

	if (m_currentStar->isLocalViewActive()) {
		createThrusterParticles();
	}
}

void Spaceship::update(Star* currentStar, const AllianceList& alliances, Faction* faction) {
	if (m_disabled) return;
	
	m_currentStar = currentStar;
	
	if (!m_dead && m_health <= 0.0f) {
		m_dead = true;
		if (!m_diesSilently) {
			m_currentStar->addAnimation(Animation("EXPLOSION", getPos()));
			Sounds::playSoundLocal("data/sound/boom1.wav", m_currentStar, getPos(), 25, 1.0f + Random::randFloat(-0.5f, 0.5f));
		}
	}

	if (m_AIEnabled) {
		if (!m_orders.empty()) {
			m_orders.front()->executing = true;
			if (m_orders.front()->execute(*this, *currentStar, alliances, faction)) {
				m_orders.pop_front();
			}
		}
		else if (m_weapons.size() > 0) {
			// Attack enemies in system

			attack(currentStar, alliances);
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
	
	if (isHeavy()) {
		if (dist > closeArea) {
			rotateTo(angle);
			float angleDiff = std::abs(m_facingAngle - angle);
			if (angleDiff < 5.0f) {
				if (dist > farArea) {
					accelerate(m_maxAcceleration);
				}
				else if (dist > closeArea) {
					keepSpeed(10.0f);
				}
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

	if (nowSpeed > speed) {
		accelerate(-(std::clamp(m_maxAcceleration, 0.0f, 0.5f) / 5.0f));
	}
	else if (nowSpeed < speed) {
		accelerate(m_maxAcceleration / 5.0f);
	}
}

Spaceship::JumpState Spaceship::jump(JumpPoint* point) {
	if (!flyTo(point->getPos())) {
		return Spaceship::JumpState::TRAVEL;
	}
	else {
		if (m_percentJumpDriveCharged >= 100.0f) {
			point->jumpShipThrough(this, m_currentStar);
			m_percentJumpDriveCharged = 0.0f;
			Sounds::playSoundLocal("data/sound/woosh1.wav", m_currentStar, getPos(), 100, 1.0f + Random::randFloat(-0.5f, 0.5f));
			Sounds::playSoundLocal("data/sound/woosh4.wav", point->getConnectedOtherStar(), getPos(), 100, 1.0f + Random::randFloat(-0.5f, 0.5f));
			m_currentStar = point->getConnectedOtherStar();
			return Spaceship::JumpState::DONE;
		}
		else {
			m_percentJumpDriveCharged += 1.0f;
			return Spaceship::JumpState::JUMPING;
		}
	}
}

std::vector<Spaceship*> Spaceship::findEnemyShips(const AllianceList& alliances) {
	std::vector<std::unique_ptr<Spaceship>>& allShips = m_currentStar->getSpaceships();
	std::vector<Spaceship*> enemies;
	for (auto& s : allShips) {
		if (!alliances.isAllied(s->getAllegiance(), m_allegiance)) {
			enemies.push_back(s.get());
		}
	}
	return enemies;
}

std::vector<Spaceship*> Spaceship::findEnemyCombatShips(const AllianceList& alliances) {
	std::vector<std::unique_ptr<Spaceship>>& allShips = m_currentStar->getSpaceships();
	std::vector<Spaceship*> enemies;
	for (auto& s : allShips) {
		if (!alliances.isAllied(s->getAllegiance(), m_allegiance) && !s->isCivilian()) {
			enemies.push_back(s.get());
		}
	}
	return enemies;
}

void Spaceship::fireAt(Spaceship* target, int weaponIdx) {
	m_weapons[weaponIdx].fireAt(getPos(), getAllegiance(), target->getPos(), m_currentStar);
}

void Spaceship::smartFireAt(Unit* target, int weaponIdx) {
	sf::Vector2f targetPos = target->getPos();
	sf::Vector2f targetVel = target->getVelocity();
	float dist = Math::distance(getPos(), targetPos);
	
	float hyp = std::sqrt(std::pow(targetVel.x, 2) + std::pow(targetVel.y, 2));
	float projectileSpeed = 10.0f;
	
	sf::Vector2f predictPos(targetPos.x + targetVel.x * (projectileSpeed * hyp), targetPos.y + targetVel.y * (projectileSpeed * hyp));

	m_weapons[weaponIdx].fireAt(getPos(), getAllegiance(), predictPos, m_currentStar);
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

void Spaceship::captureCurrentStar(Faction* faction, const AllianceList& alliances) {
	if (!alliances.isAllied(m_currentStar->getAllegiance(), m_allegiance)) {
		if (findEnemyCombatShips(alliances).size() == 0) {
			m_currentStar->factionTakeOwnership(faction);
			m_currentStar->createSpaceship("CLAIM_SHIP", getPos(), m_allegiance, m_collider.getColor());
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

std::vector<Building*> Spaceship::findEnemyBuildings(const AllianceList& alliances) {
	std::vector<Building*> buildings;
	for (auto& b : m_currentStar->getBuildings()) {
		if (!alliances.isAllied(b->getAllegiance(), m_allegiance)) {
			buildings.push_back(b.get());
		}
	}
	return buildings;
}

void Spaceship::attackRandomEnemyBuilding(std::vector<Building*>& enemyBuildings, bool urgent) {
	int randIndex;
	if (enemyBuildings.size() > 1) {
		randIndex = Random::randInt(0, enemyBuildings.size() - 1);
	}
	else {
		randIndex = 0;
	}
	if (enemyBuildings.size() > 0) {
		if (urgent) addOrderFront(AttackOrder(enemyBuildings[randIndex]));
		else addOrder(AttackOrder(enemyBuildings[randIndex]));
	}
}

void Spaceship::reinitOrdersAfterLoad(Constellation* constellation) {
	for (auto& order : m_orders) {
		order->reinitAfterLoad(constellation);
	}
}

bool Spaceship::isPlanetAttackShip() const {
	for (const Weapon& weapon : m_weapons) {
		if (weapon.getProjectile().canInvadePlanets() || weapon.getProjectile().canOrbitallyBombard()) {
			return true;
		}
	}
	return false;
}

Spaceship* Spaceship::findClosestEnemyCombatShip(Star* star, const AllianceList& alliances) {
	Spaceship* closest = nullptr;
	float closestDist = 0.0f;
	for (Spaceship* ship : star->getEnemyCombatShips(m_allegiance, alliances)) {
		float dist = Math::distance(getPos(), ship->getPos());
		if (dist < closestDist || closest == nullptr) {
			closest = ship;
			closestDist = dist;
		}
	}
	return closest;
}

Spaceship* Spaceship::findClosestEnemyShip(Star* star, const AllianceList& alliances) {
	Spaceship* closest = nullptr;
	float closestDist = 0.0f;
	for (auto& ship : star->getSpaceships()) {
		if (!alliances.isAllied(ship->getAllegiance(), m_allegiance)) {
			float dist = Math::distance(getPos(), ship->getPos());
			if (dist < closestDist || closest == nullptr) {
				closest = ship.get();
				closestDist = dist;
			}
		}
	}
	return closest;
}

bool Spaceship::attack(Star* star, const AllianceList& alliances, bool urgent) {
	Spaceship* enemy = nullptr;
	if (!m_pirate) {
		enemy = findClosestEnemyCombatShip(star, alliances);
	}
	else {
		enemy = findClosestEnemyShip(star, alliances);
	}

	if (enemy != nullptr) {
		if (urgent) addOrderFront(AttackOrder(enemy, m_fighterAI));
		else addOrder(AttackOrder(enemy, m_fighterAI));
		return true;
	}
	else {
		std::vector<Building*> enemyBuildings = findEnemyBuildings(alliances);
		if (enemyBuildings.size() > 0) {
			attackRandomEnemyBuilding(enemyBuildings, urgent);
			return true;
		}
		else {
			return false;
		}
	}
}

std::string Spaceship::getName() {
	auto& table = TOMLCache::getTable("data/objects/spaceships.toml");

	return table[m_type]["name"].value_or("Unknown");
}

void Spaceship::onDeath(Star* currentStar) {
	auto& table = TOMLCache::getTable("data/objects/spaceships.toml");
	std::string onDeathScript = table[m_type]["onDeathScript"].value_or("");

	if (onDeathScript != "") {
		Script::RunScript(onDeathScript);
		Script::CallFunction<void>("onUnitDeath", *currentStar, *this);
	}
}

void Spaceship::createThrusterParticles() {
	for (sf::Vector2f thrusterPos : m_thrusterPoints) {
		sf::Vector2f pos = m_sprite.getTransform().transformPoint(thrusterPos);

		m_currentStar->getParticleSystem().createParticle({ 10, {0.0f, 0.0f} }, pos, { 255, 204, 179 }, m_thrusterSize);
	}
}