#pragma once
#include "Spaceship.h"
#include "Brain.h"

class Constellation;
class Player;

class Faction {
public:
	Faction(Constellation* constellation);

	void spawnAtRandomStar();

	void addOwnedSystem(Star* star);

	void makeCapitol(Star* star);

	void update();

	sf::Color getColor() { return m_color; }

	//Faction(const Faction&) = delete;

	Star* getCapitol() { return m_capitol; }

	int getID() { return m_id; }

	std::vector<Spaceship*>& getShips() { return m_ships; }

	std::vector<Star*>& getOwnedStars() { return m_ownedSystems; }

	Constellation* getConstellation() { return m_constellation; }

	template <typename T>
	void giveAllShipsOrder(const T order) {
		for (Spaceship* s : m_ships) {
			s->addOrder(order);
		}
	}

	void createShip(Spaceship ship);

	void controlByPlayer(Player& player);

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

