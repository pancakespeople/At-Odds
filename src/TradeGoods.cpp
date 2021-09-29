#include "gamepch.h"
#include "TradeGoods.h"
#include "Planet.h"
#include "Faction.h"
#include "Random.h"
#include "Star.h"

void TradeGoods::addSupply(const std::string& item, float num) {
	m_items[item].supply += num;
}

float TradeGoods::removeSupply(const std::string& item, float num) {
	m_items[item].supply -= num;
	m_items[item].demand = num;

	if (m_items[item].supply < 0.0f) {
		float deficit = std::abs(m_items[item].supply);
		
		m_items[item].supply = 0.0f;
		m_items[item].demand += deficit;
		
		return deficit;
	}
	return 0.0f;
}

std::string TradeGoods::getContentString() const {
	std::string str;
	for (auto& pair : m_items) {
		str += pair.first + " - " + std::to_string(pair.second.supply) + " - " + std::to_string(pair.second.demand) + "\n";
	}
	return str;
}

void TradeGoods::update(Star* currentStar, Faction* faction, Planet* planet) {
	if (m_ticksUntilUpdate == 0) {
		float waterHarvest = planet->getColony().getPopulation() * 0.025f * planet->getWater();
		addSupply("Water", waterHarvest);

		float waterConsumption = planet->getColony().getPopulation() * 0.01f;
		removeSupply("Water", waterConsumption);

		float foodHarvest = planet->getColony().getPopulation() * 0.01f * planet->getHabitability() * planet->getColony().getBuildingEffects("foodProductionMultiplier");
		addSupply("Food", foodHarvest);

		float foodConsumption = planet->getColony().getPopulation() * 0.05f;
		removeSupply("Food", foodConsumption);

		if (faction != nullptr) {
			if (Random::randBool()) {
				spawnSpaceTruck(currentStar, faction, planet);
			}
		}

		m_ticksUntilUpdate = 1000;
	}
	else m_ticksUntilUpdate--;
}

std::vector<std::pair<std::string, float>> TradeGoods::getSurplusGoods() const {
	std::vector<std::pair<std::string, float>> surplusGoods;
	for (auto& item : m_items) {
		if (item.second.supply > item.second.demand * 2.0f) {
			surplusGoods.push_back(std::pair<std::string, float>(item.first, item.second.supply - item.second.demand));
		}
	}
	return surplusGoods;
}

std::vector<std::pair<std::string, float>> TradeGoods::getDeficitGoods() const {
	std::vector<std::pair<std::string, float>> deficitGoods;
	for (auto& item : m_items) {
		if (item.second.demand > item.second.supply) {
			deficitGoods.push_back(std::pair<std::string, float>(item.first, item.second.demand));
		}
	}
	return deficitGoods;
}

bool TradeGoods::hasDeficits() const {
	for (auto& item : m_items) {
		if (item.second.demand > item.second.supply) {
			return true;
		}
	}
	return false;
}

void TradeGoods::spawnSpaceTruck(Star* currentStar, Faction* faction, Planet* planet) {
	std::vector<Planet*> deficitPlanets;
	for (Planet& p : currentStar->getPlanets()) {
		if (p.getColony().getTradeGoods().hasDeficits() && p.getColony().getAllegiance() == faction->getID()) {
			deficitPlanets.push_back(&p);
		}
	}

	while (deficitPlanets.size() > 0) {
		int rndPlanetIndex = Random::randInt(0, deficitPlanets.size() - 1);
		for (auto surplus : getSurplusGoods()) {
			for (auto deficit : deficitPlanets[rndPlanetIndex]->getColony().getTradeGoods().getDeficitGoods()) {
				if (deficit.first == surplus.first) {
					Spaceship* truck = currentStar->createSpaceship(
						std::make_unique<Spaceship>("SPACE_TRUCK", planet->getPos(), currentStar, faction->getID(), faction->getColor())
					);
					truck->addOrder(InteractWithPlanetOrder(deficitPlanets[rndPlanetIndex], currentStar));
					truck->addOrder(DieOrder(true));

					DEBUG_PRINT("Spawned space truck in " << currentStar->getName());
					return;
				}
			}
		}
		deficitPlanets.erase(deficitPlanets.begin() + rndPlanetIndex);
	}
}