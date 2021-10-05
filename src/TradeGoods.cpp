#include "gamepch.h"
#include "TradeGoods.h"
#include "Planet.h"
#include "Faction.h"
#include "Random.h"
#include "Star.h"
#include "Util.h"

void TradeGoods::addSupply(const std::string& item, float num) {
	m_items[item].supply += num;
}

float TradeGoods::removeSupply(const std::string& item, float num) {
	m_items[item].supply -= num;
	m_items[item].demand = (m_items[item].demand + num) / 2.0f;

	if (m_items[item].supply < 0.0f) {
		float deficit = std::abs(m_items[item].supply);
		
		m_items[item].supply = 0.0f;
		m_items[item].demand += deficit;
		
		return deficit;
	}
	return 0.0f;
}

std::string TradeGoods::getContentString(Planet& planet) const {
	const toml::table& table = TOMLCache::getTable("data/objects/tradegoods.toml");
	std::string str;
	for (auto& pair : m_items) {
		str += table[pair.first]["name"].value_or(std::string("Unknown")) + " - " + Util::cutOffDecimal(pair.second.supply, 2) + " - " + Util::cutOffDecimal(pair.second.demand, 2)
			+ " - $" + Util::cutOffDecimal(calcPrice(pair.first), 2) + "\n";
	}
	return str;
}

void TradeGoods::update(Star* currentStar, Faction* faction, Planet* planet) {
	if (faction != nullptr) {
		if (m_spaceTruckTimer == 0) {
			spawnSpaceTruck(currentStar, faction, planet);
			m_spaceTruckTimer = 500;
		}
		else {
			m_spaceTruckTimer--;
		}
	}
	
	if (m_ticksUntilUpdate == 0) {
		float waterHarvest = planet->getColony().getPopulation() * 0.025f * planet->getWater();
		addSupply("WATER", waterHarvest);

		float waterConsumption = planet->getColony().getPopulation() * 0.01f;
		removeSupply("WATER", waterConsumption);

		float foodHarvest = planet->getColony().getPopulation() * 0.01f * planet->getHabitability() * planet->getColony().getBuildingEffects("foodProductionMultiplier");
		addSupply("FOOD", foodHarvest);

		float foodConsumption = planet->getColony().getPopulation() * 0.008f;
		removeSupply("FOOD", foodConsumption);

		// Production from resources
		for (Resource& resource : planet->getResources()) {
			if (resource.type == "COMMON_ORE") {
				float industrialGoodsProduction = planet->getColony().getPopulation() * 0.02f * resource.abundance;
				addSupply("INDUSTRIAL_GOODS", industrialGoodsProduction);
			}
			if (resource.type == "UNCOMMON_ORE") {
				float consumerGoodsProduction = planet->getColony().getPopulation() * 0.01f * resource.abundance;
				addSupply("CONSUMER_GOODS", consumerGoodsProduction);
			}
			if (resource.type == "RARE_ORE") {
				float luxuryGoodsProduction = planet->getColony().getPopulation() * 0.001f * resource.abundance;
				addSupply("LUXURY_GOODS", luxuryGoodsProduction);
			}
		}

		m_ticksUntilUpdate = 1000;
	}
	else m_ticksUntilUpdate--;
}

std::vector<std::pair<std::string, float>> TradeGoods::getSurplusGoods() const {
	std::vector<std::pair<std::string, float>> surplusGoods;
	for (auto& item : m_items) {
		if (item.second.supply > item.second.demand) {
			surplusGoods.push_back(std::pair<std::string, float>(item.first, item.second.supply - item.second.demand));
		}
	}
	return surplusGoods;
}

std::vector<std::pair<std::string, float>> TradeGoods::getDeficitGoods(float demandMultiplier) const {
	std::vector<std::pair<std::string, float>> deficitGoods;
	for (auto& item : m_items) {
		if (item.second.demand * demandMultiplier > item.second.supply) {
			deficitGoods.push_back(std::pair<std::string, float>(item.first, item.second.demand));
		}
	}
	return deficitGoods;
}

bool TradeGoods::hasDeficits(float demandMultiplier) const {
	for (auto& item : m_items) {
		if (item.second.demand * demandMultiplier > item.second.supply) {
			return true;
		}
	}
	return false;
}

void TradeGoods::spawnSpaceTruck(Star* currentStar, Faction* faction, Planet* planet) {
	std::vector<std::pair<Planet*, Star*>> deficitPlanets;
	for (Star* star : faction->getOwnedStars()) {
		for (Planet& p : star->getPlanets()) {
			if (p.getColony().getTradeGoods().hasDeficits(4.0f) && p.getColony().getAllegiance() == faction->getID()) {
				deficitPlanets.push_back(std::pair<Planet*, Star*>(&p, star));
			}
		}
	}

	while (deficitPlanets.size() > 0) {
		int rndPlanetIndex = Random::randInt(0, deficitPlanets.size() - 1);
		for (auto surplus : getSurplusGoods()) {
			for (auto deficit : deficitPlanets[rndPlanetIndex].first->getColony().getTradeGoods().getDeficitGoods(4.0f)) {
				if (deficit.first == surplus.first) {
					Spaceship* truck;
					float maxItems;
					float wantedItems;
					std::string truckType;
					bool ready = false;
					
					// Decide which type of truck to spawn
					if (surplus.second > 100.0f) {
						truckType = "BIG_SPACE_TRUCK";
						maxItems = 1000.0f;
						wantedItems = std::min(surplus.second, maxItems);
						ready = m_items[surplus.first].supply - wantedItems * 3.0f > m_items[surplus.first].demand;
					}
					// Fall back to smaller trucks if needed
					if (!ready) {
						truckType = "SPACE_TRUCK";
						maxItems = 100.0f;
						wantedItems = std::min(surplus.second, maxItems);
						ready = m_items[surplus.first].supply - wantedItems * 3.0f > m_items[surplus.first].demand;
					}

					if (ready) {
						truck = currentStar->createSpaceship(
							std::make_unique<Spaceship>(truckType, planet->getPos(), currentStar, faction->getID(), faction->getColor())
						);
						TradeMod mod;
						mod.addItem(surplus.first, std::min(surplus.second, maxItems));
						removeSupply(surplus.first, std::min(surplus.second, maxItems));

						truck->addMod(mod);
						truck->addOrder(TravelOrder(deficitPlanets[rndPlanetIndex].second));
						truck->addOrder(InteractWithPlanetOrder(deficitPlanets[rndPlanetIndex].first, deficitPlanets[rndPlanetIndex].second));
						truck->addOrder(TravelOrder(currentStar));
						truck->addOrder(InteractWithPlanetOrder(planet, currentStar));
						truck->addOrder(DieOrder(true));

						return;
					}
				}
			}
		}
		deficitPlanets.erase(deficitPlanets.begin() + rndPlanetIndex);
	}
}

float TradeGoods::getSupply(const std::string& item) {
	if (m_items.count(item) == 0) return 0.0f;
	return m_items[item].supply;
}

float TradeGoods::calcPrice(const std::string& item) const {
	const toml::table& table = TOMLCache::getTable("data/objects/tradegoods.toml");
	float price = table[item]["price"].value_or(1.0f);

	price *= m_items.at(item).demand / std::max(m_items.at(item).supply, 1.0f) + price;
	return price;
}