#pragma once

#include <map>
#include <string>

class Planet;
class Faction;
class Star;

struct SupplyDemand {
	float supply = 0.0f;
	float demand = 0.0f;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & supply;
		archive & demand;
	}
};

class TradeGoods {
public:
	TradeGoods() {}

	void addSupply(const std::string& item, float num);
	void update(Star* currentStar, Faction* faction, Planet* planet);

	// Returns the deficit, if there is one
	float removeSupply(const std::string& item, float num);
	float getSupply(const std::string& item);
	float calcPrice(const std::string& item) const;

	bool hasDeficits(float demandMultiplier = 1.0f) const;

	// Returns a pair with the item and the supply
	std::vector<std::pair<std::string, float>> getSurplusGoods() const;
	// Returns a pair with the item and the demand
	std::vector<std::pair<std::string, float>> getDeficitGoods(float demandMultiplier = 1.0f) const;
	std::string getContentString(Planet& planet) const;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_items;
		archive & m_ticksUntilUpdate;
		archive & m_spaceTruckTimer;
	}

	void spawnSpaceTruck(Star* currentStar, Faction* faction, Planet* planet);

	std::map<std::string, SupplyDemand> m_items;
	int m_ticksUntilUpdate = 1000;
	int m_spaceTruckTimer = 500;
};