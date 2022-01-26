#include "gamepch.h"
#include "AllianceList.h"

void AllianceList::add(int allianceID, int allegiance) {
	m_alliances[allianceID].insert(allegiance);
}

bool AllianceList::isAllied(int allegiance1, int allegiance2) const {
	for (auto& pair : m_alliances) {
		if (pair.second.count(allegiance1) > 0 && pair.second.count(allegiance2) > 0) {
			return true;
		}
	}

	return false;
}

std::unordered_set<int> AllianceList::getAllies(int allegiance) const {
	for (auto& pair : m_alliances) {
		if (pair.second.count(allegiance) > 0) {
			return pair.second;
		}
	}

	return std::unordered_set<int>();
}

std::unordered_set<int> AllianceList::getAlliance(int allianceID) const {
	return m_alliances.at(allianceID);
}