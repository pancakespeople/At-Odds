#include "gamepch.h"
#include "Building.h"
#include "TextureCache.h"
#include "Random.h"
#include "Animation.h"
#include "Star.h"
#include "Sounds.h"
#include "Math.h"

const std::unordered_map<Building::BUILDING_TYPE, std::string> Building::texturePaths = {
	{BUILDING_TYPE::OUTPOST, "data/art/outpost.png"}
};

Building::Building(BUILDING_TYPE type, Star* star, sf::Vector2f pos, int allegiance, sf::Color color) {
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

		m_health = 1000.0f;

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

void Building::update() {
	if (!m_dead && m_health <= 0.0f) {
		m_dead = true;
		m_currentStar->addAnimation(Animation(Animation::ANIMATION_TYPE::EXPLOSION, getPos()));
		Sounds::playSoundLocal("data/sound/boom1.wav", m_currentStar, 25, 1.0f + Random::randFloat(-0.5f, 0.5f));
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

BuildingPrototype::BuildingPrototype(Building::BUILDING_TYPE type) {
	m_type = type;
	sf::Texture& texture = TextureCache::getTexture(Building::texturePaths.at(Building::BUILDING_TYPE::OUTPOST));
	m_sprite.setTexture(texture);
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
}

void BuildingPrototype::draw(sf::RenderWindow& window) {
	window.draw(m_sprite);
}