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
#include "Renderer.h"

Building::Building(const std::string& type, Star* star, sf::Vector2f pos, Faction* faction, bool built) {
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");

	assert(table.contains(type));

	int allegiance;
	sf::Color factionColor;

	if (faction == nullptr) {
		allegiance = -1;
		factionColor = Faction::neutralColor;
	}
	else {
		allegiance = faction->getID();
		factionColor = faction->getColor();
	}

	m_sprite.setTexture(TextureCache::getTexture(table[type]["texturePath"].value_or("")));
	m_sprite.setScale(table[type]["scale"].value_or(1.0f), table[type]["scale"].value_or(1.0f));
	m_maxHealth = table[type]["health"].value_or(100.0f);
	m_constructionSpeedMultiplier = table[type]["constructionSpeedMultiplier"].value_or(1.0f);
	m_type = type;
	m_collider = Collider(pos, factionColor, m_sprite.getLocalBounds().width * m_sprite.getScale().x / 1.5f * table[type]["colliderScale"].value_or(1.0f));
	m_rotates = table[type]["rotates"].value_or(true);

	if (table[type].as_table()->contains("weapons")) {
		for (auto& weapon : *table[type]["weapons"].as_array()) {
			auto val = weapon.value<std::string>();
			if (val) {
				if (val.value() == "$RAND_WEAPON") {
					auto weapons = faction->getBuildingWeapons();
					if (weapons.size() > 0) {
						int rndIndex = Random::randInt(0, weapons.size() - 1);
						addWeapon(Weapon(weapons[rndIndex].type));
					}
				}
				else {
					Weapon weapon(val.value());
					weapon.getProjectile().setLife(weapon.getProjectile().getLife() * table[type]["weaponLifeMultiplier"].value_or(1.0f));
					weapon.getProjectile().setSpeed(weapon.getProjectile().getSpeed() * table[type]["weaponSpeedMultiplier"].value_or(1.0f));

					addWeapon(weapon);
				}
			}
		}
	}

	if (table[type].as_table()->contains("mods")) {
		for (auto& mod : *table[type]["mods"].as_array()) {
			std::string val = mod.value_or("");
			
			if (val == "FighterBay") {
				addMod(FighterBayMod(this, star, allegiance, factionColor));
			}
			else if (val == "Factory") {
				addMod(FactoryMod());
			}
			else if (val == "Habitat") {
				addMod(HabitatMod(100000, 1000000, true));
			}
			else if (val == "Science") {
				addMod(ScienceMod());
			}
			else if (val == "PirateBase") {
				addMod(PirateBaseMod());
			}
		}
	}

	if (table[type].as_table()->contains("color")) {
		auto arr = *table[type]["color"].as_array();
		int r = arr[0].value_or(0);
		int g = arr[1].value_or(0);
		int b = arr[2].value_or(0);
		m_sprite.setColor(sf::Color(r, g, b));
	}

	m_rotateToTarget = table[type]["rotateToTarget"].value_or(false);

	m_sprite.setPosition(pos);
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
	
	if (m_rotates) {
		m_sprite.setRotation(Random::randFloat(0.0f, 360.0f));
	}
	
	m_allegiance = allegiance;
	m_currentStar = star;

	if (built) {
		enableAllMods();
		m_constructionPercent = 100.0f;
		m_health = m_maxHealth;
		onBuild();
	}
	else {
		m_constructionPercent = 1.0f;
		m_health = m_maxHealth * (m_constructionPercent / 100.0f);
		m_resourceCost = getResourceCost(type);
	}
}

void Building::draw(Renderer& renderer, sf::Vector2f mouseWorldPos) {
	if (m_constructionPercent < 100.0f) {
		sf::Color oldColor = m_sprite.getColor();
		sf::Color newColor = oldColor;

		newColor.a = std::max(10.0f, 255 * (m_constructionPercent / 100.0f));

		m_sprite.setColor(newColor);
		renderer.draw(m_sprite);
		m_sprite.setColor(oldColor);
	}
	else {
		renderer.draw(m_sprite);
	}
	
	m_collider.draw(renderer, mouseWorldPos);

	if (m_selected) {
		drawSelectionCircle(renderer);
		drawHealthBar(renderer);
	}

	for (Weapon& weapon : m_weapons) {
		weapon.drawFireAnimation(renderer, this);
	}
}

void Building::update(Star* currentStar, const AllianceList& alliances) {
	m_currentStar = currentStar;
	
	if (!m_dead && m_health <= 0.0f) {
		m_dead = true;
		m_currentStar->addAnimation(Animation("EXPLOSION", getPos()));
		Sounds::playSoundLocal("data/sound/boom1.wav", m_currentStar, getPos(), 25, 1.0f + Random::randFloat(-0.5f, 0.5f));
	}
	
	if (m_constructionPercent < 100.0f) {
		return;
	}
	
	if (m_rotates) m_sprite.rotate(0.1f);
	
	updateWeapons();
	attackEnemies(alliances);

	m_collider.update(getPos());
}

void Building::attackEnemies(const AllianceList& alliances) {
	if (m_weapons.size() == 0) {
		return;
	}
	if (m_attackTarget != nullptr) {
		if (m_attackTarget->isDead()) {
			m_attackTarget = nullptr;
			m_attackTargetID = 0;
		}
		else {
			float weaponRange = getLongestWeaponRange();
			float dist = Math::distance(getPos(), m_attackTarget->getPos());
			if (dist < weaponRange && m_attackTarget->getCurrentStar() == m_currentStar) {
				for (Weapon& weapon : m_weapons) {
					weapon.fireAt(getPos(), getAllegiance(), m_attackTarget->getPos(), m_currentStar);
				}

				if (m_rotateToTarget) {
					float angle = Math::angleBetween(getPos(), m_attackTarget->getPos());
					m_sprite.setRotation(-angle + 90.0f);
				}
			}
			else {
				m_attackTarget = nullptr;
				m_attackTargetID = 0;
			}
		}
	}
	else {
		std::vector<Unit*> enemyUnits = findEnemyUnits(alliances);

		if (enemyUnits.size() > 0) {
			float range = getLongestWeaponRange();
			for (Unit* unit : enemyUnits) {
				if (Math::distance(getPos(), unit->getPos()) < range) {
					m_attackTarget = unit;
					m_attackTargetID = unit->getID();
					break;
				}
			}
		}
	}
}

void Building::construct(const Spaceship& constructor, Faction& faction) {
	if (m_constructionPercent >= 100.0f) {
		return;
	}

	float percentIncrease = constructor.getConstructionSpeed() / m_maxHealth;
	float percentChange = (m_constructionPercent + percentIncrease * m_constructionSpeedMultiplier) - m_constructionPercent;

	std::unordered_map<std::string, float> cost = m_resourceCost;
	for (auto& item : cost) {
		item.second *= percentChange / 100.0f;
	}

	if (faction.canSubtractResources(cost)) {

		faction.subtractResources(cost);
		m_constructionPercent += percentIncrease * m_constructionSpeedMultiplier;

		m_health = m_maxHealth * (m_constructionPercent / 100.0f);

		if (m_constructionPercent >= 100.0f) {
			m_resourceCost.clear();
			onBuild();
		}
	}
}

bool Building::checkBuildCondition(const std::string& type, sf::Vector2f pos, float radius, Star* star, Faction* faction, bool player) {
	if (star == nullptr) {
		return false;
	}
	else if (player) {
		if (star->numAllies(faction->getID()) == 0) {
			return false;
		}
	}
	
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");

	Collider collider(pos, sf::Color::Black, radius);

	// Check collision
	for (auto& building : star->getBuildings()) {
		if (collider.isCollidingWith(building->getCollider())) {
			return false;
		}
	}

	if (table[type].as_table()->contains("numPerStar")) {
		return star->numAlliedBuildings(faction->getID(), type) < table[type]["numPerStar"].value_or(1);
	}

	if (table[type]["scienceLab"].value_or(false)) {
		return star->numAlliedBuildings(faction->getID(), type) < faction->getScienceLabMax(star);
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

void Building::reinitAfterLoad(Star* star) {
	m_currentStar = star;
	m_attackTarget = star->getUnitByID(m_attackTargetID);
}

std::string Building::getName() {
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");
	return table[m_type]["name"].value_or("Unknown");
}

BuildingPrototype::BuildingPrototype(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");

	assert(table.contains(type));

	m_type = type;
	sf::Texture& texture = TextureCache::getTexture(table[type]["texturePath"].value_or(""));
	m_sprite.setTexture(texture);
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
	m_sprite.setScale(table[type]["scale"].value_or(1.0f), table[type]["scale"].value_or(1.0f));
	m_radius = m_sprite.getLocalBounds().width * m_sprite.getScale().x / 1.5f;
}

void BuildingPrototype::draw(sf::RenderWindow& window, Star* currentStar, Faction* playerFaction) {
	if (Building::checkBuildCondition(m_type, m_sprite.getPosition(), m_radius, currentStar, playerFaction, true)) {
		m_sprite.setColor(sf::Color(0, 200, 0));
		window.draw(m_sprite);
	}
	else {
		m_sprite.setColor(sf::Color(200, 0, 0));
		window.draw(m_sprite);
	}
}

bool BuildingPrototype::meetsDisplayRequirements(const std::string& type, Faction* faction) {
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");

	assert(table.contains(type));

	if (table[type].as_table()->contains("weaponRequirements")) {
		for (auto& weapon : *table[type]["weaponRequirements"].as_array()) {
			if (!faction->hasWeapon(weapon.value_or(""))) {
				return false;
			}
		}
	}

	if (table[type]["requiresTech"].value_or("") != "") {
		if (!faction->hasResearchedTech(table[type]["requiresTech"].value_or(""))) {
			return false;
		}
	}

	return true;
}

void Building::onBuild() {
	for (auto& mod : m_mods) {
		mod->onBuild(this, m_currentStar);
	}
}

std::unordered_map<std::string, float> Building::getResourceCost(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");
	assert(table.contains(type));

	std::unordered_map<std::string, float> cost;

	if (table[type]["cost"].as_array() != nullptr) {
		for (int i = 0; i < table[type]["cost"].as_array()->size(); i++) {
			std::string resourceType = table[type]["cost"][i][0].value_or("");
			cost[resourceType] = table[type]["cost"][i][1].value_or(0.0f);
		}
	}

	// Add weapon cost to building cost
	if (table[type].as_table()->contains("weapons")) {
		for (auto& weapon : *table[type]["weapons"].as_array()) {
			auto val = weapon.value<std::string>();
			if (val) {
				if (val.value() != "$RAND_WEAPON") {
					DesignerWeapon weaponDesign(val.value());

					for (auto& item : weaponDesign.resourceCost) {
						if (cost.count(item.first) == 0) {
							cost[item.first] = 0.0f;
						}

						cost[item.first] += item.second;
					}
				}
			}
		}
	}

	return cost;
}