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

	text += "\n";
	text += "Research Points: " + Util::cutOffDecimal(getRequiredResearchPoints(), 2) + "\n";
	
	float timeToResearch = getTimeToResearch(faction);

	if (timeToResearch == -1.0f) text += "Time to research: Infinity (You have no research point production)\n";
	else text += "Time to research: " + Util::secondsToTime(timeToResearch) + "\n";
	
	auto buildings = getUnlockedBuildings();

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

	return text;
}

std::vector<std::string> Tech::getUnlockedBuildings() const {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	std::vector<std::string> buildings;

	auto* arr = table[m_type]["addsColonyBuildings"].as_array();

	if (arr != nullptr) {
		for (auto& building : *arr) {
			buildings.push_back(building.value_or(""));
		}
	}

	return buildings;
}