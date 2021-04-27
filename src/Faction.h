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
	Constellation* m_constellation;
	
	sf::Color m_color;
	
	std::vector<Star*> m_ownedSystems;
	
	Star* m_capitol = nullptr;
	
	int m_ticksUntilNextAIAction;
	
	int m_id;

	std::vector<Spaceship*> m_ships;

	Brain m_ai;

	int m_freeShipTimer = 2000;

	int m_numFreeShips = 0;

	bool m_aiEnabled = true;
};

