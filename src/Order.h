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

	Order() {}
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {}

};

class FlyToOrder : public Order {
public:
	FlyToOrder(sf::Vector2f pos) { m_pos = pos; }

	virtual bool execute(Spaceship* ship) override;

	sf::Vector2f getTargetPos() { return m_pos; }
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
		archive& m_pos;
	}

	FlyToOrder() {}

	sf::Vector2f m_pos;
};

//BOOST_CLASS_EXPORT_GUID(FlyToOrder, "FlyToOrder")

class JumpOrder : public Order {
public:
	JumpOrder(JumpPoint* point, bool attackEnemies = false) { m_jumpPoint = point; m_attackEnemies = attackEnemies; }

	virtual bool execute(Spaceship* ship) override;

	JumpPoint* getTargetJumpPoint() { return m_jumpPoint; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
		archive& m_jumpPoint;
		archive& m_attackEnemies;
	}

	JumpOrder() {}

	JumpPoint* m_jumpPoint;
	bool m_attackEnemies;
};

//BOOST_CLASS_EXPORT_GUID(JumpOrder, "JumpOrder")

class AttackOrder : public Order {
public:
	AttackOrder(Unit* target);

	virtual bool execute(Spaceship* ship) override;

	Unit* getTargetShip() { return m_target; }
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
		archive& m_target;
		archive& m_frustration;
		archive& m_lastEnemyHealth;
	}

	AttackOrder() {}

	Unit* m_target;
	float m_frustration;
	float m_lastEnemyHealth;
};

//BOOST_CLASS_EXPORT_GUID(AttackOrder, "AttackOrder")

class TravelOrder : public Order {
public:
	TravelOrder(Star* star);

	virtual bool execute(Spaceship* ship) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
		archive& m_pathFound;
		archive& m_endStar;
		archive& m_path;
	}

	TravelOrder() {}

	bool m_pathFound = false;
	Star* m_endStar;
	std::list<Star*> m_path;
};

//BOOST_CLASS_EXPORT_GUID(TravelOrder, "TravelOrder")

class InteractWithBuildingOrder : public Order {
public:
	InteractWithBuildingOrder(Building* building);

	virtual bool execute(Spaceship* ship) override;

	Building* getTargetBuilding() { return m_building; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
		archive& m_building;
	}

	InteractWithBuildingOrder() {}
	
	Building* m_building;
};

//BOOST_CLASS_EXPORT_GUID(InteractWithBuildingOrder, "InteractWithBuildingOrder")