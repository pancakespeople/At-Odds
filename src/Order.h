#pragma once
#include <SFML/Graphics.hpp>

class Spaceship;
class JumpPoint;
class Star;
class Unit;
class Building;

class Order {
public:
	// Should return true if order is finished
	virtual bool execute(Spaceship* ship) { return true; }
};

class FlyToOrder : public Order {
public:
	FlyToOrder(sf::Vector2f pos) { m_pos = pos; }

	virtual bool execute(Spaceship* ship) override;

	sf::Vector2f getTargetPos() { return m_pos; }
private:
	sf::Vector2f m_pos;
};

class JumpOrder : public Order {
public:
	JumpOrder(JumpPoint* point, bool attackEnemies = false) { m_jumpPoint = point; m_attackEnemies = attackEnemies; }

	virtual bool execute(Spaceship* ship) override;

	JumpPoint* getTargetJumpPoint() { return m_jumpPoint; }

private:
	JumpPoint* m_jumpPoint;
	bool m_attackEnemies;
};

class AttackOrder : public Order {
public:
	AttackOrder(Unit* target);

	virtual bool execute(Spaceship* ship) override;

	Unit* getTargetShip() { return m_target; }
private:
	Unit* m_target;
	float m_frustration;
	float m_lastEnemyHealth;
};

class TravelOrder : public Order {
public:
	TravelOrder(Star* star);

	virtual bool execute(Spaceship* ship) override;

private:
	bool m_pathFound = false;
	Star* m_endStar;
	std::list<Star*> m_path;
};

class InteractWithBuildingOrder : public Order {
public:
	InteractWithBuildingOrder(Building* building);

	virtual bool execute(Spaceship* ship) override;

	Building* getTargetBuilding() { return m_building; }

private:
	Building* m_building;
};