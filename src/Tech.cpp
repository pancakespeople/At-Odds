#include "gamepch.h"
#include "Tech.h"
#include "TOMLCache.h"

Tech::Tech(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	assert(table.contains(type));
	m_type = type;
}

std::string Tech::getName() const {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	return table[m_type]["name"].value_or("Unknown");
}

std::string Tech::getDescription() const {
	const toml::table& table = TOMLCache::getTable("data/objects/tech.toml");
	return table[m_type]["description"].value_or("?");
}

void Tech::addResearchPercent(float percent) {
	if (m_researchPercent + percent > 100.0f) m_researchPercent = 100.0f;
	else m_researchPercent += percent;
}