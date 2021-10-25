#include "gamepch.h"
#include "Tech.h"
#include "TOMLCache.h"

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