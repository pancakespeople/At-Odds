#include "gamepch.h"
#include "Tech.h"
#include "TOMLCache.h"
#include "Util.h"
#include "Colony.h"
#include "Faction.h"

Tech::Tech(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	assert(table.contains(type));
	m_type = type;
	m_researchPointsRequired = table[type]["researchRequired"].value_or(1000.0f);
}

std::string Tech::getName() const {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	return table[m_type]["name"].value_or("Unknown");
}

std::string Tech::getDescription() const {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	return table[m_type]["description"].value_or("?");
}

void Tech::addResearchPoints(float research) {
	if (m_researchPoints + research > m_researchPointsRequired) m_researchPoints = m_researchPointsRequired;
	else m_researchPoints += research;
}

bool Tech::hasFlag(const std::string& flag) const {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	return table[m_type][flag].value_or(false);
}

float Tech::getTimeToResearch(Faction* faction) const {
	float researchPointProduction = faction->getResearchPointProduction();
	if (researchPointProduction == 0.0f) return -1.0f;
	return (m_researchPointsRequired - m_researchPoints) / researchPointProduction / 60.0f;
}

std::string Tech::getExtendedDescription(Faction* faction) const {
	std::string text = getDescription();
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");

	text += "\n";
	text += "Research Points: " + Util::cutOffDecimal(getRequiredResearchPoints(), 2) + "\n";
	
	float timeToResearch = getTimeToResearch(faction);

	if (timeToResearch == -1.0f) text += "Time to research: Infinity (You have no research point production)\n";
	else text += "Time to research: " + Util::secondsToTime(timeToResearch) + "\n";
	
	auto buildings = getUnlocked("addsColonyBuildings");

	if (buildings.size() > 0) {
		text += "Unlocks buildings: ";
		for (int i = 0; i < buildings.size(); i++) {
			ColonyBuilding building(buildings[i]);
			text += building.getName();
			if (i != buildings.size() - 1) {
				text += ", ";
			}
		}
		text += "\n";
	}

	auto chassis = getUnlocked("addsChassis");
	if (chassis.size() > 0) {
		text += "Unlocks chassis: ";
		for (int i = 0; i < chassis.size(); i++) {
			Spaceship::DesignerChassis theChassis(chassis[i]);
			text += theChassis.name;
			if (i != chassis.size() - 1) {
				text += ", ";
			}
		}
		text += "\n";
	}

	auto weapons = getUnlocked("addsWeapons");
	if (weapons.size() > 0) {
		text += "Unlocks weapons: ";
		for (int i = 0; i < weapons.size(); i++) {
			Spaceship::DesignerWeapon weapon(weapons[i]);
			text += weapon.name;
			if (i != weapons.size() - 1) {
				text += ", ";
			}
		}
		text += "\n";
	}

	if (table[m_type]["unlockRandomWeapon"].value_or(false)) {
		text += "Unlocks a random weapon\n";
	}

	return text;
}

std::string Tech::getCategory() const {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	return table[m_type]["category"].value_or("");
}

std::vector<std::string> Tech::getUnlocked(const std::string& key) const {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	std::vector<std::string> things;

	auto* arr = table[m_type][key].as_array();

	if (arr != nullptr) {
		for (auto& thing : *arr) {
			things.push_back(thing.value_or(""));
		}
	}

	return things;
}