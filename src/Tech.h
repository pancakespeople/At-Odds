#pragma once
#include <string>

#include "Designs.h"

class Faction;

class Tech {
public:
	Tech(const std::string& type);
	std::string getName() const;
	std::string getDescription() const;
	std::string getExtendedDescription(Faction* faction) const;
	std::string getType() const { return m_type; }
	std::string getCategory() const;
	std::vector<std::string> getUnlocked(const std::string& key) const;
	
	float getResearchPercent() const { return m_researchPoints / m_researchPointsRequired * 100.0f; }
	float getRequiredResearchPoints() const { return m_researchPointsRequired; }
	float getResearchPoints() const { return m_researchPoints; }
	float getTimeToResearch(Faction* faction) const;

	void addResearchPoints(float research);
	void setResearching(bool researching) { m_researching = researching; }
	void setRequiredResearchPoints(float points) { m_researchPointsRequired = points; }
	void resetResearchPoints() { m_researchPoints = 0.0f; }
	
	static Tech generateWeaponUpgradeTech(const DesignerWeapon& weapon);
	
	bool isResearched() const { return m_researchPoints >= m_researchPointsRequired; }
	bool isResearching() const { return m_researching; }
	bool hasFlag(const std::string& flag) const;

	bool Tech::operator==(const Tech& right) {
		return getType() == right.getType();
	}

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_type;
		archive & m_researchPointsRequired;
		archive & m_researchPoints;
		archive & m_researching;
	}
	
	Tech() {}

	std::string m_type;
	float m_researchPointsRequired = 0.0f;
	float m_researchPoints = 0.0f;
	bool m_researching = false;
};