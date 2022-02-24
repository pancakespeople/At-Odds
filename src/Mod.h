#pragma once

#include "Designs.h"

class Unit;
class Star;
class Faction;
class Planet;
class Spaceship;
class Constellation;
class AllianceList;

class Mod {
public:
	Mod() {}

	// Faction can be nullptr, others cant
	virtual void update(Unit& unit, Star& currentStar, Faction* faction, const AllianceList& alliances) {}
	virtual void interactWithPlanet(Unit*, Planet* planet, Star* star) {}
	virtual void onUnitDeath(Star* currentStar) {}
	virtual void openGUI(tgui::ChildWindow::Ptr window, Faction* faction);
	virtual void onShipInteract(Spaceship* ship) {}
	virtual void onBuild(Unit* unit, Star* currentStar) {}

	void enable() { m_enabled = true; }
	void disable() { m_enabled = false; }
	void setEnabled(bool enabled) { m_enabled = enabled; }

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

	virtual void update(Unit& unit, Star& currentStar, Faction* faction, const AllianceList& alliances) override;
	virtual std::string getInfoString() override;
	virtual void openGUI(tgui::ChildWindow::Ptr window, Faction* faction) override;
	virtual void onShipInteract(Spaceship* ship);

	void updateDesigns(Faction* faction);
	void setBuildAll(bool build);
	void setBuild(const std::string& name, bool build);

	float getBuildSpeedMultiplier();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Mod>(*this);
		archive & m_shipBuildData;
		archive & m_weaponsStockpile;
		archive & m_checkForWeaponsTimer;
	}
	
	struct ShipBuildData {
		ShipBuildData(const std::string shipName) { this->shipName = shipName; }
		
		bool selected = false;
		bool build = false;
		bool resourcesSubtracted = false;
		bool continuous = false;
		float progressPercent = 0.0f;
		int amount = 1;
		float buildTimeMultiplier = 1.0f;
		std::string shipName;

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& archive, const unsigned int version) {
			archive & build;
			archive & resourcesSubtracted;
			archive & progressPercent;
			archive & continuous;
			archive & amount;
			archive & buildTimeMultiplier;
			archive & shipName;
		}

		ShipBuildData() = default;
	};

	std::deque<ShipBuildData> m_shipBuildData;
	tgui::ProgressBar::Ptr m_buildProgressBar;
	tgui::Group::Ptr m_shipWidgets;
	tgui::Label::Ptr m_armamentsLabel;
	tgui::Label::Ptr m_buildSpeedLabel;
	tgui::ListBox::Ptr m_designsListBox;

	float m_weaponsStockpile = 0.0f;
	int m_checkForWeaponsTimer = 1000;

	void updateDesignsListBox(int selectedIndex);
};

class FighterBayMod : public Mod {
public:
	enum class FIGHTER_STATUS {
		FIGHTING,
		DOCKED,
		RETURNING
	};
	
	FighterBayMod(const Unit* unit, Star* star, int allegiance, sf::Color color);
	
	virtual void update(Unit& unit, Star& currentStar, Faction* faction, const AllianceList& alliances) override;
	virtual std::string getInfoString() override;
	virtual void onUnitDeath(Star* currentStar) override;

	void launchFighters(Star* currentStar);
	void recallFighters(Star* currentStar, Unit* unit);
	void dockReturningFighters(Star* currentStar, Unit* unit);
	void constructNewFighter(Star* currentStar, Unit* unit, Faction* faction);
	void killAllFighters(Star* currentStar);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Mod>(*this);
		archive & m_fighterShipIds;
		archive & m_fighterStatus;
		archive & m_ticksToNextFighter;
	}
	
	FighterBayMod() {}

	std::vector<unsigned int> m_fighterShipIds;
	FIGHTER_STATUS m_fighterStatus = FIGHTER_STATUS::DOCKED;
	const int m_maxFighters = 4;
	int m_ticksToNextFighter = 2500;
};

class HabitatMod : public Mod {
public:
	HabitatMod(int population, int maxPopulation, bool spawnsSpaceBus);
	
	virtual void update(Unit& unit, Star& currentStar, Faction* faction, const AllianceList& alliances) override;
	virtual void interactWithPlanet(Unit* unit, Planet* planet, Star* star) override;

	virtual std::string getInfoString() override;

	static int calcBusTickTimer(int population);
	static Star* findBusStarDestination(Star* currentStar, Faction* faction);
	static Planet* findBusPlanetDestination(int allegiance, Star* targetStar, Planet* avoidPlanet = nullptr);
	static void createSpaceBus(Unit* unit, Star* currentStar, Star* targetStar, Planet* targetPlanet);
	static void createSpaceBus(sf::Vector2f pos, int allegiance, sf::Color color, Star* currentStar, Star* targetStar, Planet* targetPlanet);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Mod>(*this);
		archive & m_population;
		archive & m_ticksToNextGrowth;
		archive & m_popCap;
		archive & m_ticksToNextBus;
		archive & m_growthRate;
		archive & m_spawnsSpaceBus;
	}

	HabitatMod() {}
	
	Faction* m_faction = nullptr;

	int m_population = 100000;
	int m_ticksToNextGrowth = 1000;
	int m_popCap = 1000000;
	int m_ticksToNextBus = 500;

	float m_growthRate = 0.01f;

	bool m_spawnsSpaceBus = false;
};

class TradeMod : public Mod {
public:
	TradeMod() {}

	void addItem(const std::string& item, float num, float price);
	void setItem(const std::string& item, float num) { m_goods[item] = num; }
	virtual void interactWithPlanet(Unit* unit, Planet* planet, Star* star) override;

	const std::map<std::string, float>& getGoods() const { return m_goods; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Mod>(*this);
		archive & m_goods;
		archive & m_money;
	}

	std::map<std::string, float> m_goods;
	float m_money = 0.0f;
};

class ScienceMod : public Mod {
public:
	ScienceMod() = default;

	virtual void update(Unit& unit, Star& currentStar, Faction* faction, const AllianceList& alliances) override;
	virtual void openGUI(tgui::ChildWindow::Ptr window, Faction* faction) override;

	inline static const float resourceConsumption = 5.0f;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Mod>(*this);
		archive & m_research;
		archive & m_checkResearchTimer;
		archive & m_resourceType;
		archive & m_researching;
	}

	void updateInfoLabel(tgui::ChildWindow::Ptr window);

	int m_checkResearchTimer = 0;
	float m_research = 0.0f;
	std::string m_resourceType = "COMMON_ORE";
	bool m_researching = true;
};

class PirateBaseMod : public Mod {
public:
	PirateBaseMod() = default;
	
	virtual void update(Unit& unit, Star& currentStar, Faction* faction, const AllianceList& alliances) override;
	int getTheftAllegiance() { return m_theftAllegiance; }
	void findTheftAllegiance(Star* currentStar, Constellation* constellation);
	void stealDesignFrom(Faction* faction);
	void addDesign(const DesignerShip& ship) { m_stolenDesigns.push_back(ship); }
	void setTheftAllegiance(int allegiance) { m_theftAllegiance = allegiance; }
	bool hasDesign(const DesignerShip& design);
	virtual void onBuild(Unit* unit, Star* currentStar) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Mod>(*this);
		archive & m_nextShipPercent;
		archive & m_theftAllegiance;
		archive & m_stolenDesigns;
		archive & m_lifetimeTicks;
		archive & m_spawnNewBaseTime;
	}

	int m_theftAllegiance = -1;
	int m_lifetimeTicks = 0;
	float m_nextShipPercent = 0.0f;
	std::deque<DesignerShip> m_stolenDesigns;
	int m_spawnNewBaseTime = 20000.0f;
};