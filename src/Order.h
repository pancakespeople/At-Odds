#pragma once
#include <SFML/Graphics.hpp>

#include "Designs.h"

class Spaceship;
class JumpPoint;
class Star;
class Unit;
class Building;
class EffectsEmitter;
class Planet;
class Constellation;
class Renderer;
class AllianceList;
class Asteroid;
class Faction;

class Order {
public:
	// Should return true if order is finished
	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) { return true; }

	virtual void draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) {}
	virtual void reinitAfterLoad(Constellation* constellation) {}
	
	// Returns a pair, first value is true if there IS a destination, second is the coordinates of the destination
	virtual std::pair<bool, sf::Vector2f> getDestinationPos(Star* currentStar) { return std::pair<bool, sf::Vector2f>(false, sf::Vector2f()); }

	Order() {}

	bool executing = false;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & executing;
	}

};

class FlyToOrder : public Order {
public:
	FlyToOrder(sf::Vector2f pos) { m_pos = pos; }

	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;
	virtual void draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) override;
	virtual std::pair<bool, sf::Vector2f> getDestinationPos(Star* currentStar) override { return std::pair<bool, sf::Vector2f>(true, m_pos); }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_pos;
	}

	FlyToOrder() {}

	sf::Vector2f m_pos;
};

class JumpOrder : public Order {
public:
	JumpOrder(JumpPoint* point, bool attackEnemies = false);

	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;

	virtual void draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_jumpPointID;
		archive & m_attackEnemies;
	}

	JumpOrder() {}

	JumpPoint* m_jumpPoint = nullptr;
	unsigned int m_jumpPointID = 0;
	bool m_attackEnemies = false;
};

class AttackOrder : public Order {
public:
	AttackOrder(Unit* target, bool aggressive = false);

	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;
	virtual void draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) override;

	unsigned int getTargetShipID() { return m_targetID; }

	virtual std::pair<bool, sf::Vector2f> getDestinationPos(Star* currentStar) override;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_targetID;
		archive & m_frustration;
		archive & m_lastEnemyHealth;
		archive & m_aggressive;
	}

	AttackOrder() {}

	unsigned int m_targetID = 0;
	Unit* m_target = nullptr;
	float m_frustration = 0.0f;
	float m_lastEnemyHealth = 100.0f;
	bool m_aggressive = false;
};

class TravelOrder : public Order {
public:
	TravelOrder(Star* star);

	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;
	virtual void reinitAfterLoad(Constellation* constellation) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_pathFound;
		archive & m_endStarID;
		archive & m_pathIDs;
	}

	TravelOrder() {}

	bool m_pathFound = false;
	
	Star* m_endStar = nullptr;
	uint32_t m_endStarID = 0;

	std::list<Star*> m_path;
	std::list<uint32_t> m_pathIDs;
};

class InteractWithBuildingOrder : public Order {
public:
	InteractWithBuildingOrder(Building* building);

	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;
	virtual void draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) override;
	virtual std::pair<bool, sf::Vector2f> getDestinationPos(Star* currentStar) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_buildingID;
	}

	InteractWithBuildingOrder() {}
	
	unsigned int m_buildingID = 0;
	Building* m_building = nullptr;
};

class InteractWithPlanetOrder : public Order {
public:
	InteractWithPlanetOrder(Planet* planet, Star* star);

	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;
	virtual void draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) override;

	virtual std::pair<bool, sf::Vector2f> getDestinationPos(Star* currentStar) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_planetID;
	}
	
	InteractWithPlanetOrder() {}

	unsigned int m_planetID = 0;
	Planet* m_planet = nullptr;
};

class DieOrder : public Order {
public:
	DieOrder(bool silently = false);

	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_dieSilently;
	}

	bool m_dieSilently = false;
};

class InteractWithUnitOrder : public Order {
public:
	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;
	virtual void draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) override;
	virtual std::pair<bool, sf::Vector2f> getDestinationPos(Star* currentStar);

	InteractWithUnitOrder(Unit* unit);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_unitID;
	}

	InteractWithUnitOrder() {}

	uint32_t m_unitID = 0;
	Unit* m_unit = nullptr;
};

class EstablishPirateBaseOrder : public Order {
public:
	virtual bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;

	EstablishPirateBaseOrder(sf::Vector2f pos, int theftAllegiance, const std::deque<DesignerShip>& designs);
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_pos;
		archive & m_theftAllegiance;
		archive & m_designs;
	}

	EstablishPirateBaseOrder() = default;
	
	sf::Vector2f m_pos;
	int m_theftAllegiance = -1;
	std::deque<DesignerShip> m_designs;
};

class MineAsteroidOrder : public Order {
public:
	bool execute(Spaceship& ship, Star& currentStar, const AllianceList& alliances, Faction* faction) override;
	void draw(Renderer& renderer, const sf::Vector2f& shipPos, Star* currentStar) override;
	std::pair<bool, sf::Vector2f> getDestinationPos(Star* currentStar) override;

	MineAsteroidOrder(Asteroid* asteroid);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Order>(*this);
		archive & m_asteroidID;
	}

	MineAsteroidOrder() = default;

	uint32_t m_asteroidID = 0;
	Asteroid* m_asteroid = nullptr;
};