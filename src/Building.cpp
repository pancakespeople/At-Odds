#include "gamepch.h"
#include "Building.h"
#include "TextureCache.h"
#include "Random.h"
#include "Animation.h"
#include "Star.h"
#include "Sounds.h"
#include "Math.h"
#include "Mod.h"
#include "Player.h"

const std::unordered_map<Building::BUILDING_TYPE, std::string> Building::texturePaths = {
	{BUILDING_TYPE::OUTPOST, "data/art/outpost.png"},
	{BUILDING_TYPE::LASER_TURRET, "data/art/laserturret.png"},
	{BUILDING_TYPE::MACHINE_GUN_TURRET, "data/art/machinegunturret.png"},
	{BUILDING_TYPE::GAUSS_TURRET, "data/art/gaussturret.png"},
	{BUILDING_TYPE::SHIP_FACTORY, "data/art/factory.png"},
	{BUILDING_TYPE::SPACE_HABITAT, "data/art/spacehabitat.png"}
};

Building::Building(BUILDING_TYPE type, Star* star, sf::Vector2f pos, int allegiance, sf::Color color, bool built) {
	switch (type) {
	case BUILDING_TYPE::OUTPOST:
		m_sprite.setTexture(TextureCache::getTexture(texturePaths.at(BUILDING_TYPE::OUTPOST)));
		m_sprite.setScale(sf::Vector2f(2.0f, 2.0f));

		if (Random::randBool()) {
			for (int i = 0; i < 4; i++) {
				addWeapon(Weapon(Weapon::WEAPON_TYPE::LONG_RANGE_LASER_GUN));
			}
		}
		else {
			for (int i = 0; i < 4; i++) {
				addWeapon(Weapon(Weapon::WEAPON_TYPE::LONG_RANGE_MACHINE_GUN));
			}
		}

		m_collider = Collider(pos, color, m_sprite.getLocalBounds().width * m_sprite.getScale().x / 1.5f);
		addMod(FighterBayMod(this, star, allegiance, color));

		m_health = 1000.0f;

		break;
	case BUILDING_TYPE::LASER_TURRET:
		m_sprite.setTexture(TextureCache::getTexture(texturePaths.at(BUILDING_TYPE::LASER_TURRET)));

		addWeapon(Weapon(Weapon::WEAPON_TYPE::LASER_GUN));

		m_health = 150.0f;
		break;
	case BUILDING_TYPE::MACHINE_GUN_TURRET:
		m_sprite.setTexture(TextureCache::getTexture(texturePaths.at(BUILDING_TYPE::MACHINE_GUN_TURRET)));

		addWeapon(Weapon(Weapon::WEAPON_TYPE::MACHINE_GUN));

		m_health = 150.0f;
		break;
	case BUILDING_TYPE::GAUSS_TURRET:
		m_sprite.setTexture(TextureCache::getTexture(texturePaths.at(BUILDING_TYPE::GAUSS_TURRET)));

		addWeapon(Weapon(Weapon::WEAPON_TYPE::GAUSS_CANNON));

		m_health = 350.0f;
		m_constructionSpeedMultiplier = 0.5f;
		break;
	case BUILDING_TYPE::SHIP_FACTORY:
		m_sprite.setTexture(TextureCache::getTexture(texturePaths.at(BUILDING_TYPE::SHIP_FACTORY)));
		m_sprite.setScale(sf::Vector2f(2.0f, 2.0f));

		addMod(FactoryMod());

		m_health = 2500.0f;
		m_constructionSpeedMultiplier = 1.5f;
		break;
	case BUILDING_TYPE::SPACE_HABITAT:
		m_sprite.setTexture(TextureCache::getTexture(texturePaths.at(BUILDING_TYPE::SPACE_HABITAT)));
		m_sprite.setScale(sf::Vector2f(2.0f, 2.0f));

		addMod(HabitatMod());

		m_health = 1000.0f;
		break;
	}

	m_collider = Collider(pos, color, m_sprite.getLocalBounds().width * m_sprite.getScale().x / 1.5f);

	m_sprite.setPosition(pos);
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
	m_sprite.setRotation(Random::randFloat(0.0f, 360.0f));
	
	m_allegiance = allegiance;
	m_type = type;
	m_currentStar = star;

	if (built) {
		enableAllMods();
		m_constructionPercent = 100.0f;
	}
	else {
		m_constructionPercent = 0.0f;
	}
}

void Building::draw(sf::RenderWindow& window) {
	if (m_constructionPercent < 100.0f) {
		sf::Color oldColor = m_sprite.getColor();
		sf::Color newColor = oldColor;

		newColor.a = std::max(10.0f, 255 * (m_constructionPercent / 100.0f));

		m_sprite.setColor(newColor);
		window.draw(m_sprite);
		m_sprite.setColor(oldColor);
	}
	else {
		window.draw(m_sprite);
	}
	window.draw(m_collider);
}

void Building::update(Star* currentStar, Faction& faction) {
	m_currentStar = currentStar;
	
	if (!m_dead && m_health <= 0.0f) {
		m_dead = true;
		m_currentStar->addAnimation(Animation(Animation::ANIMATION_TYPE::EXPLOSION, getPos()));
		Sounds::playSoundLocal("data/sound/boom1.wav", m_currentStar, 25, 1.0f + Random::randFloat(-0.5f, 0.5f));
	}
	
	updateMods(currentStar, faction);
	
	if (m_constructionPercent < 100.0f) {
		return;
	}
	
	updateWeapons();
	attackEnemies();

	m_sprite.rotate(0.1f);
}

void Building::attackEnemies() {
	if (m_weapons.size() == 0) {
		return;
	}
	if (m_attackTarget != nullptr) {
		if (m_attackTarget->isDead()) {
			m_attackTarget = nullptr;
		}
		else {
			float weaponRange = getLongestWeaponRange();
			float dist = Math::distance(getPos(), m_attackTarget->getPos());
			if (dist < weaponRange && m_attackTarget->getCurrentStar() == m_currentStar) {
				for (Weapon& weapon : m_weapons) {
					weapon.fireAt(this, m_attackTarget->getPos(), m_currentStar);
				}
			}
			else {
				m_attackTarget = nullptr;
			}
		}
	}
	else {
		std::vector<Unit*> enemyUnits = findEnemyUnits();

		if (enemyUnits.size() > 0) {
			float range = getLongestWeaponRange();
			for (Unit* unit : enemyUnits) {
				if (Math::distance(getPos(), unit->getPos()) < range) {
					m_attackTarget = unit;
					break;
				}
			}
		}
	}
}

void Building::construct(const Spaceship* constructor) {
	if (m_constructionPercent >= 100.0f) {
		return;
	}

	float percentIncrease = constructor->getConstructionSpeed() / m_health;
	m_constructionPercent += percentIncrease * m_constructionSpeedMultiplier;
}

bool Building::checkBuildCondition(BUILDING_TYPE type, const Star* star, int allegiance, bool player) {
	if (star == nullptr) {
		return false;
	}
	else if (player) {
		if (star->numAllies(allegiance) == 0) {
			return false;
		}
	}
	
	switch (type) {
	case BUILDING_TYPE::OUTPOST: // Only one allowed per star, per faction
		return !star->containsBuildingType(type, true, allegiance);
		break;
	case BUILDING_TYPE::SHIP_FACTORY: // Only one allowed per star, per faction
		return !star->containsBuildingType(type, true, allegiance);
		break;
	default:
		return true;
	}
}

BuildingPrototype::BuildingPrototype(Building::BUILDING_TYPE type) {
	m_type = type;
	sf::Texture& texture = TextureCache::getTexture(Building::texturePaths.at(type));
	m_sprite.setTexture(texture);
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
}

void BuildingPrototype::draw(sf::RenderWindow& window, const Star* currentStar, const Player& player) {
	if (Building::checkBuildCondition(m_type, currentStar, player.getFaction(), true)) {
		m_sprite.setColor(sf::Color(0, 200, 0));
		window.draw(m_sprite);
	}
	else {
		m_sprite.setColor(sf::Color(200, 0, 0));
		window.draw(m_sprite);
	}
}