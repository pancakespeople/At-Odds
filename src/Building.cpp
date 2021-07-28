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
#include "TOMLCache.h"

Building::Building(const std::string& type, Star* star, sf::Vector2f pos, Faction* faction, bool built) {
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");

	assert(table.contains(type));

	m_sprite.setTexture(TextureCache::getTexture(table[type]["texturePath"].value_or("")));
	m_sprite.setScale(table[type]["scale"].value_or(1.0f), table[type]["scale"].value_or(1.0f));
	m_health = table[type]["health"].value_or(100.0f);
	m_constructionSpeedMultiplier = table[type]["constructionSpeedMultiplier"].value_or(1.0f);
	m_name = table[type]["name"].value_or("");
	m_collider = Collider(pos, faction->getColor(), m_sprite.getLocalBounds().width * m_sprite.getScale().x / 1.5f);

	if (table[type].as_table()->contains("weapons")) {
		for (auto& weapon : *table[type]["weapons"].as_array()) {
			auto& val = weapon.value<std::string>();
			if (val) {
				if (val.value() == "$RAND_WEAPON") {
					auto& weapons = faction->getWeapons();
					if (weapons.size() > 0) {
						int rndIndex = Random::randInt(0, weapons.size() - 1);
						addWeapon(Weapon(weapons[rndIndex].type));
					}
				}
				else {
					addWeapon(Weapon(val.value()));
				}
			}
		}
	}

	if (table[type].as_table()->contains("mods")) {
		for (auto& mod : *table[type]["mods"].as_array()) {
			std::string val = mod.value_or("");
			
			if (val == "FighterBay") {
				addMod(FighterBayMod(this, star, faction->getID(), faction->getColor()));
			}
			else if (val == "Factory") {
				addMod(FactoryMod());
			}
			else if (val == "Habitat") {
				addMod(HabitatMod(100000, 1000000, true));
			}
		}
	}

	m_sprite.setPosition(pos);
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
	m_sprite.setRotation(Random::randFloat(0.0f, 360.0f));
	
	m_allegiance = faction->getID();
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

void Building::update(Star* currentStar) {
	m_currentStar = currentStar;
	
	if (!m_dead && m_health <= 0.0f) {
		m_dead = true;
		m_currentStar->addAnimation(Animation("EXPLOSION", getPos()));
		Sounds::playSoundLocal("data/sound/boom1.wav", m_currentStar, getPos(), 25, 1.0f + Random::randFloat(-0.5f, 0.5f));
	}
	
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

bool Building::checkBuildCondition(const std::string& type, const Star* star, int allegiance, bool player) {
	if (star == nullptr) {
		return false;
	}
	else if (player) {
		if (star->numAllies(allegiance) == 0) {
			return false;
		}
	}
	
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");
	
	if (table[type]["onePerStar"].value_or(false)) {
		return !star->containsBuildingName(table[type]["name"].value_or(""), true, allegiance);
	}

	return true;
}

std::string Building::getInfoString() {
	std::string info;
	for (auto& mod : m_mods) {
		info += mod->getInfoString();
		info += "\n";
	}
	return info;
}

BuildingPrototype::BuildingPrototype(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");

	assert(table.contains(type));

	m_type = type;
	sf::Texture& texture = TextureCache::getTexture(table[type]["texturePath"].value_or(""));
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
