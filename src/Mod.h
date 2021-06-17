#pragma once

class Unit;
class Star;
class Faction;
class Planet;

class Mod {
public:
	Mod() {}

	// Faction can be nullptr, others cant
	virtual void update(Unit* unit, Star* currentStar, Faction* faction) {}
	virtual void interactWithPlanet(Planet* planet) {}
	void enable() { m_enabled = true; }
	void disable() { m_enabled = false; }

	bool isEnabled() { return m_enabled; }

	virtual std::string getInfoString() { return ""; }

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

	virtual void update(Unit* unit, Star* currentStar, Faction* faction) override;
	virtual std::string getInfoString() override;

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
	
	virtual void update(Unit* unit, Star* currentStar, Faction* faction) override;
	virtual std::string getInfoString() override;

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

class HabitatMod : public Mod {
public:
	HabitatMod(int population, int maxPopulation, bool spawnsSpaceBus);
	
	virtual void update(Unit* unit, Star* currentStar, Faction* faction) override;
	virtual void interactWithPlanet(Planet* planet) override;

	virtual std::string getInfoString() override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		boost::serialization::base_object<Mod>(*this);
		archive & m_population;
		archive & m_ticksToNextGrowth;
		archive & m_popCap;
		archive & m_ticksToNextBus;
		archive & m_growthRate;
		archive & m_spawnsSpaceBus;
	}

	HabitatMod() {}
	
	int m_population = 100000;
	int m_ticksToNextGrowth = 1000;
	int m_popCap = 1000000;
	int m_ticksToNextBus = 3500;

	float m_growthRate = 0.01f;

	bool m_spawnsSpaceBus = false;
};