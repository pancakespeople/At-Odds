#pragma once

class AllianceList {
public:
	void add(int allianceID, int allegiance);
	bool isAllied(int allegiance1, int allegiance2) const;
	std::unordered_set<int> getAllies(int allegiance) const;
	std::unordered_set<int> getAlliance(int allianceID) const;

private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_alliances;
	}

	std::unordered_map<int, std::unordered_set<int>> m_alliances;
};