#pragma once

class Unit;
class Star;
class Faction;

class Mod {
public:
	Mod() {}

	virtual void update(Unit* unit, Star* currentStar, Faction& faction) {}
	void enable() { m_enabled = true; }
	void disable() { m_enabled = false; }

	bool isEnabled() { return m_enabled; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_enabled;
	}

	bool m_enabled = false;
};

class FactoryMod : public Mod {
public:

	virtual void update(Unit* unit, Star* currentStar, Faction& faction) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Mod>(*this);
		archive & m_ticksToNextShip;
		archive & m_numShips;
	}
	
	int m_ticksToNextShip = 2000;
	int m_numShips = 0;
};

class FighterBayMod : public Mod {
public:
	enum class FIGHTER_STATUS {
		FIGHTING,
		DOCKED,
		RETURNING
	};
	
	FighterBayMod(const Unit* unit, Star* star, int allegiance, sf::Color color);
	
	virtual void update(Unit* unit, Star* currentStar, Faction& faction) override;

	void launchFighters(Star* currentStar);
	void recallFighters(Star* currentStar, Unit* unit);
	void dockReturningFighters(Star* currentStar, Unit* unit);
	void constructNewFighter(Star* currentStar, Unit* unit);
	void killAllFighters(Star* currentStar);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Mod>(*this);
		archive & m_fighterShipIds;
		archive & m_fighterStatus;
		archive & m_ticksToNextFighter;
	}
	
	FighterBayMod() {}

	std::vector<unsigned int> m_fighterShipIds;
	FIGHTER_STATUS m_fighterStatus = FIGHTER_STATUS::DOCKED;
	const int m_maxFighters = 4;
	int m_ticksToNextFighter = 1000;
};