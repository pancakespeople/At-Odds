#pragma once
#include <SFML/Graphics.hpp>

class Spaceship;
class JumpPoint;
class Star;
class Unit;
class Building;
class EffectsEmitter;
class Planet;

class Order {
public:
	// Should return true if order is finished
	virtual bool execute(Spaceship* ship, Star* currentStar) { return true; }

	virtual void draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) {}

	Order() {}
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {}

};

class FlyToOrder : public Order {
public:
	FlyToOrder(sf::Vector2f pos) { m_pos = pos; }

	virtual bool execute(Spaceship* ship, Star* currentStar) override;

	virtual void draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
		archive & m_pos;
	}

	FlyToOrder() {}

	sf::Vector2f m_pos;
};

class JumpOrder : public Order {
public:
	JumpOrder(JumpPoint* point, bool attackEnemies = false);

	virtual bool execute(Spaceship* ship, Star* currentStar) override;

	virtual void draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
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

	virtual bool execute(Spaceship* ship, Star* currentStar) override;

	virtual void draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) override;

	unsigned int getTargetShipID() { return m_targetID; }
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
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

	virtual bool execute(Spaceship* ship, Star* currentStar) override;

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
	Star* m_endStar = nullptr;
	std::list<Star*> m_path;
};

class InteractWithBuildingOrder : public Order {
public:
	InteractWithBuildingOrder(Building* building);

	virtual bool execute(Spaceship* ship, Star* currentStar) override;

	virtual void draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
		archive& m_buildingID;
	}

	InteractWithBuildingOrder() {}
	
	unsigned int m_buildingID = 0;
	Building* m_building = nullptr;
};

class InteractWithPlanetOrder : public Order {
public:
	InteractWithPlanetOrder(Planet* planet, Star* star);

	virtual bool execute(Spaceship* ship, Star* currentStar) override;

	virtual void draw(sf::RenderWindow& window, EffectsEmitter& emitter, const sf::Vector2f& shipPos) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
		archive & m_planetID;
	}
	
	InteractWithPlanetOrder() {}

	unsigned int m_planetID = 0;
	Planet* m_planet = nullptr;
};

class DieOrder : public Order {
public:
	DieOrder(bool silently = false);

	virtual bool execute(Spaceship* ship, Star* currentStar) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Order>(*this);
		archive & m_dieSilently;
	}

	bool m_dieSilently = false;
};
