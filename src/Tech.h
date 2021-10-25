#pragma once
#include <string>

class Tech {
public:
	Tech(const std::string& type);
	std::string getName() const;
	std::string getDescription() const;
	std::string getType() const { return m_type; }
	
	float getResearchPercent() const { return m_researchPoints / m_researchPointsRequired * 100.0f; }
	
	void addResearchPoints(float research);
	void setResearching(bool researching) { m_researching = researching; }
	
	bool isResearched() const { return m_researchPoints >= m_researchPointsRequired; }
	bool isResearching() const { return m_researching; }
	bool hasFlag(const std::string& flag) const;

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