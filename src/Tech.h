#pragma once
#include <string>

class Tech {
public:
	Tech(const std::string& type);
	std::string getName() const;
	std::string getDescription() const;
	std::string getType() const { return m_type; }
	
	float getResearchPercent() const { return m_researchPercent; }
	
	void addResearchPercent(float percent);
	void setResearching(bool researching) { m_researching = researching; }
	
	bool isResearched() const { return m_researchPercent >= 100.0f; }
	bool isResearching() const { return m_researching; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_type;
		archive & m_researchPercent;
		archive & m_researching;
	}
	
	Tech() {}

	std::string m_type;
	float m_researchPercent = 0.0f;
	bool m_researching = false;
};