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

float TradeGoods::removeSupply(const std::string& item, float num, bool noDemand) {
	m_items[item].supply -= num;
	if (!noDemand) m_items[item].demand = (m_items[item].demand + num) / 2.0f;

	if (m_items[item].supply < 0.0f) {
		float deficit = std::abs(m_items[item].supply);
		
		m_items[item].supply = 0.0f;
		if (!noDemand) m_items[item].demand += deficit;
		
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
			m_spaceTruckTimer = 4000 * planet->getColony().getBuildingEffects("truckSpawnTimeMultiplier");
		}
		else {
			m_spaceTruckTimer--;
		}
	}
	
	if (m_ticksUntilUpdate == 0) {
		if (planet->getColony().hasBuildingOfType("WATER_PUMP", true)) {
			float waterHarvest = planet->getColony().getPopulation() * 0.025f * planet->getWater();
			addSupply("WATER", waterHarvest);
		}

		float waterConsumption = planet->getColony().getPopulation() * 0.01f;
		float waterDeficit = removeSupply("WATER", waterConsumption);
		if (waterDeficit > 0.0f) planet->getColony().removeStability(waterDeficit / 2000.0f);

		if (planet->getColony().hasBuildingOfType("FARMING", true)) {
			float foodHarvest = planet->getColony().getPopulation() * 0.01f * planet->getHabitability() * planet->getColony().getBuildingEffects("foodProductionMultiplier");
			addSupply("FOOD", foodHarvest);
		}

		float foodConsumption = planet->getColony().getPopulation() * 0.008f;
		float foodDeficit = removeSupply("FOOD", foodConsumption);
		if (foodDeficit > 0.0f) planet->getColony().removeStability(foodDeficit / 3000.0f);

		// Production from resources
		for (Resource& resource : planet->getResources()) {
			if (!resource.hidden) {
				if (resource.type == "COMMON_ORE") {
					if (planet->getColony().hasBuildingOfType("FACTORY", true)) {
						float industrialGoodsProduction = planet->getColony().getPopulation() * 0.1f * resource.abundance;
						addSupply("INDUSTRIAL_GOODS", industrialGoodsProduction);
					}
				}
				if (resource.type == "UNCOMMON_ORE") {
					if (planet->getColony().hasBuildingOfType("CONSUMER_GOODS_FACTORIES", true)) {
						float consumerGoodsProduction = planet->getColony().getPopulation() * 0.085f * resource.abundance * planet->getColony().getBuildingEffects("consumerGoodsMultiplier");
						float deficit = removeSupply("INDUSTRIAL_GOODS", consumerGoodsProduction);
						addSupply("CONSUMER_GOODS", consumerGoodsProduction - deficit);
					}

					if (planet->getColony().hasBuildingOfType("WEAPONS_FACTORIES", true)) {
						float armamentsProduction = planet->getColony().getPopulation() * 0.005f * resource.abundance * planet->getColony().getBuildingEffects("armamentsMultiplier");
						float deficit = removeSupply("INDUSTRIAL_GOODS", armamentsProduction);
						addSupply("ARMAMENTS", armamentsProduction - deficit);
					}
				}
				if (resource.type == "RARE_ORE") {
					if (planet->getColony().hasBuildingOfType("WEAPONS_FACTORIES", true)) {
						float luxuryGoodsProduction = planet->getColony().getPopulation() * 0.05f * resource.abundance;
						float deficit = removeSupply("INDUSTRIAL_GOODS", luxuryGoodsProduction * 2.0f);
						addSupply("LUXURY_GOODS", luxuryGoodsProduction - deficit);
					}
				}
			}
		}

		//float wealthFactor = std::clamp((1000.0f / std::abs(planet->getColony().getWealth() - 100.0f)) - 9.0f, 1.0f, 30.0f);
		float wealthFactor = planet->getColony().getWealth() / 100.0f;
		float consumerGoodsConsumption = std::min(planet->getColony().getPopulation() * 0.001f * wealthFactor, 1000.0f);

		//wealthFactor = std::min(std::pow(2.0f, (planet->getColony().getWealth() - 100.0f) / 30.0f), 100.0f);
		wealthFactor = planet->getColony().getWealth() / 500.0f;
		float luxuryGoodsConsumption = std::min(planet->getColony().getPopulation() * 0.001f * wealthFactor, 1000.0f);

		float deficit = removeSupply("CONSUMER_GOODS", consumerGoodsConsumption);
		if (deficit > 0.0f) {
			planet->getColony().removeStability(deficit / 20000.0f);
			planet->getColony().removeWealth(deficit / 10.0f);
		}
		else planet->getColony().addStability(0.05f);

		deficit = removeSupply("LUXURY_GOODS", luxuryGoodsConsumption);
		if (deficit > 0.0f) {
			planet->getColony().removeStability(deficit / 30000.0f);
			planet->getColony().removeWealth(deficit);
		}
		else planet->getColony().addStability(0.05f);
		
		for (auto& good : m_items) {
			// Set prices and trends
			good.second.price = calcPrice(good.first);
			good.second.supplyChange = good.second.supply - m_oldItems[good.first].supply;
			good.second.demandChange = good.second.demand - m_oldItems[good.first].demand;
			good.second.priceChange = good.second.price - m_oldItems[good.first].price;

			// News events
			if (!good.second.shortage) {
				if (good.second.supply < good.second.demand) {
					good.second.shortage = true;
					
					int rnd = Random::randInt(0, 1);

					switch (rnd) {
					case 0:
						faction->addNewsEvent("There is a shortage of " + getGoodName(good.first) + " on " + planet->getName(currentStar) + ".");
						break;
					case 1:
						faction->addNewsEvent("We have run out of " + getGoodName(good.first) + " on " + planet->getName(currentStar) + "!");
						break;
					}
				}
			}
			else if (good.second.supply > good.second.demand && good.second.shortage) {
				good.second.shortage = false;

				int rnd = Random::randInt(0, 1);

				switch (rnd) {
				case 0:
					faction->addNewsEvent("The shortage of " + getGoodName(good.first) + " on " + planet->getName(currentStar) + " has been resolved.");
					break;
				case 1:
					faction->addNewsEvent("We have finally ended the shortage of " + getGoodName(good.first) + " on " + planet->getName(currentStar) + ".");
					break;
				}
			}
		}

		m_oldItems = m_items;

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
	
	// Get any planets that have any sort of deficit
	std::vector<std::pair<Planet*, Star*>> deficitPlanets;
	for (Star* star : faction->getOwnedStars()) {
		for (Planet& p : star->getPlanets()) {
			if (p.getColony().getTradeGoods().hasDeficits() && p.getColony().getAllegiance() == faction->getID() && &p != planet) {
				deficitPlanets.push_back(std::pair<Planet*, Star*>(&p, star));
			}
		}
	}

	// Sort so that planets with a higher population are first
	std::sort(deficitPlanets.begin(), deficitPlanets.end(), [](std::pair<Planet*, Star*>& a, std::pair<Planet*, Star*>& b) {
		return a.first->getColony().getPopulation() > b.first->getColony().getPopulation();
	});

	auto surplusGoods = getSurplusGoods();
	
	if (surplusGoods.size() > 0) {
		auto surplus = getSurplusGoods()[Random::randInt(0, surplusGoods.size() - 1)];

		for (auto& deficitPlanet : deficitPlanets) {
			for (auto deficit : deficitPlanet.first->getColony().getTradeGoods().getDeficitGoods()) {
				if (deficit.first == surplus.first) {
					Spaceship* truck;
					float maxItems;
					float wantedItems;
					std::string truckType;

					// Decide which type of truck to spawn
					if (surplus.second > 1000.0f) {
						truckType = "BIG_SPACE_TRUCK";
						maxItems = 10000.0f;
						wantedItems = std::min(surplus.second / 2.0f, maxItems);
					}
					// Fall back to smaller trucks if needed
					else {
						truckType = "SPACE_TRUCK";
						maxItems = 1000.0f;
						wantedItems = std::min(surplus.second / 2.0f, maxItems);
					}

					truck = currentStar->createSpaceship(truckType, planet->getPos(), faction->getID(), faction->getColor());
					TradeMod mod;
					mod.addItem(surplus.first, wantedItems, calcPrice(surplus.first));
					removeSupply(surplus.first, wantedItems, true);

					truck->addMod(mod);
					truck->addOrder(TravelOrder(deficitPlanet.second));
					truck->addOrder(InteractWithPlanetOrder(deficitPlanet.first, deficitPlanet.second));
					truck->addOrder(TravelOrder(currentStar));
					truck->addOrder(InteractWithPlanetOrder(planet, currentStar));
					truck->addOrder(DieOrder(true));

					return;
				}
			}
		}
	}
}

float TradeGoods::getSupply(const std::string& item) const {
	if (m_items.count(item) == 0) return 0.0f;
	return m_items.at(item).supply;
}

float TradeGoods::calcPrice(const std::string& item, float supplyAddition) const {
	const toml::table& table = TOMLCache::getTable("data/objects/tradegoods.toml");
	float price = table[item]["price"].value_or(1.0f);

	price *= m_items.at(item).demand / std::max(m_items.at(item).supply + supplyAddition, 1.0f) + price;
	return price;
}

float TradeGoods::sellGoods(const std::string& item, float num) {
	float revenue = (calcPrice(item) * num + calcPrice(item, num) * num) / 2.0f;
	addSupply(item, num);
	return revenue;
}

std::string TradeGoods::getGoodName(const std::string& type) {
	const toml::table& table = TOMLCache::getTable("data/objects/tradegoods.toml");
	return table[type]["name"].value_or("???");
}