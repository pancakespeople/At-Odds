#pragma once
#include "Spaceship.h"
#include "Brain.h"

class Constellation;
class Player;
class Building;

class Faction {
public:
	Faction(Constellation* constellation, int id);

	void spawnAtRandomStar();
	void addOwnedSystem(Star* star);
	void makeCapitol(Star* star);
	void update();
	void controlByPlayer(Player& player);
	void orderConstructionShipsBuild(Building* building, bool onlyIdleShips = false, bool onlyOne = false);
	void addSpaceship(Spaceship* ship) { m_ships.push_back(ship); }

	int getID() { return m_id; }
	int numUnbuiltBuildings(Star* star);
	int numIdleConstructionShips();
	
	sf::Color getColor() { return m_color; }

	std::vector<Spaceship*>& getShips() { return m_ships; }
	std::vector<Spaceship*> getConstructionShips(bool onlyIdleShips = false);
	std::vector<Star*>& getOwnedStars() { return m_ownedSystems; }

	Star* getCapitol() { return m_capitol; }
	Constellation* getConstellation() { return m_constellation; }

	template <typename T>
	void giveAllShipsOrder(const T order) {
		for (Spaceship* s : m_ships) {
			s->addOrder(order);
		}
	}

	template<typename T>
	void giveAllCombatShipsOrder(const T order) {
		for (Spaceship* s : m_ships) {
			if (s->getConstructionSpeed() == 0.0f) {
				s->addOrder(order);
			}
		}
	}

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& m_constellation;
		archive& m_capitol;
		archive& m_color;
		archive& m_ownedSystems;
		archive& m_ships;
		archive& m_ai;
		archive& m_ticksUntilNextAIAction;
		archive& m_id;
		archive& m_freeShipTimer;
		archive& m_numFreeShips;
		archive& m_aiEnabled;
	}
	
	Faction() {}

	Constellation* m_constellation;
	Star* m_capitol = nullptr;
	
	sf::Color m_color;
	std::vector<Star*> m_ownedSystems;
	std::vector<Spaceship*> m_ships;
	
	Brain m_ai;
	
	int m_ticksUntilNextAIAction;
	int m_id;
	int m_freeShipTimer = 2000;
	int m_numFreeShips = 0;
	
	bool m_aiEnabled = true;
};

