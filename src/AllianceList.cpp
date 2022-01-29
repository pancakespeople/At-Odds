#include "gamepch.h"
#include "AllianceList.h"

void AllianceList::add(int allianceID, int allegiance) {
	m_alliances[allianceID].insert(allegiance);
}

bool AllianceList::isAllied(int allegiance1, int allegiance2) const {
	if (allegiance1 == allegiance2) {
		return true;
	}

	for (auto& alliance : m_alliances) {
		if (alliance.count(allegiance1) > 0 && alliance.count(allegiance2) > 0) {
			return true;
		}
	}

	return false;
}

std::unordered_set<int> AllianceList::getAllies(int allegiance) const {
	for (auto& alliance : m_alliances) {
		if (alliance.count(allegiance) > 0) {
			return alliance;
		}
	}

	return std::unordered_set<int>({ allegiance });
}

std::unordered_set<int> AllianceList::getAlliance(int allianceID) const {
	return m_alliances.at(allianceID);
}

int AllianceList::createAlliance() {
	m_alliances.push_back(std::unordered_set<int>());
	return m_alliances.size() - 1;
}

int AllianceList::getAllianceIDOf(int allegiance) const {
	for (int i = 0; i < m_alliances.size(); i++) {
		if (m_alliances[i].count(allegiance) > 0) {
			return i;
		}
	}

	return -1;
}