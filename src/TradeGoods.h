#pragma once

#include <map>
#include <string>

class Planet;
class Faction;
class Star;

struct SupplyDemand {
	float supply = 0.0f;
	float demand = 0.0f;
	float price = 0.0f;
	float supplyChange = 1.0f;
	float demandChange = 0.0f;
	float priceChange = 0.0f;
	bool shortage = false;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & supply;
		archive & demand;
		archive & price;
		archive & supplyChange;
		archive & demandChange;
		archive & priceChange;
		archive & shortage;
	}
};

class TradeGoods {
public:
	TradeGoods() {}

	void addSupply(const std::string& item, float num);
	void update(Star* currentStar, Faction* faction, Planet* planet);

	// Returns the deficit, if there is one
	float removeSupply(const std::string& item, float num, bool noDemand = false);
	float getSupply(const std::string& item) const;
	float calcPrice(const std::string& item, float supplyAddition = 0.0f) const;
	float sellGoods(const std::string& item, float num);

	bool hasDeficits(float demandMultiplier = 1.0f) const;

	// Returns a pair with the item and the supply
	std::vector<std::pair<std::string, float>> getSurplusGoods() const;
	// Returns a pair with the item and the demand
	std::vector<std::pair<std::string, float>> getDeficitGoods(float demandMultiplier = 1.0f) const;
	std::string getContentString(Planet& planet) const;
	const std::map<std::string, SupplyDemand>& getGoods() const { return m_items; }
	std::string getGoodName(const std::string& type);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_items;
		archive & m_oldItems;
		archive & m_ticksUntilUpdate;
		archive & m_spaceTruckTimer;
	}

	void spawnSpaceTruck(Star* currentStar, Faction* faction, Planet* planet);

	std::map<std::string, SupplyDemand> m_items;
	std::map<std::string, SupplyDemand> m_oldItems;
	int m_ticksUntilUpdate = 1000;
	int m_spaceTruckTimer = 4000;
};