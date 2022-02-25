#pragma once
class Faction;
class Star;
class Brain;
class Colony;
class Planet;
class Constellation;
class AllianceList;

class SubAI {
public:
	void sleep(uint32_t ticks);
	bool sleepCheck();

	virtual void update(Faction& faction, Brain& brain, const AllianceList& alliances) = 0;

private:
	uint32_t m_sleepTime = 0;
};

class MilitaryAI : public SubAI {
public:
	enum class MilitaryState {
		ATTACKING,
		RALLYING,
		NONE,
	};
	
	virtual void update(Faction& faction, Brain& brain, const AllianceList& alliances) override;
	void reinitAfterLoad(Constellation* constellation);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_state;
		archive & m_expansionTargetID;
		archive & m_launchingAttack;
		archive & m_attackTimer;
		archive & m_attackFrustration;
		archive & m_rallyTimer;
	}
	MilitaryState m_state = MilitaryState::NONE;
	int m_stateChangeTimer = 0;

	Star* m_expansionTarget = nullptr; // Star that the AI wants to attack
	uint32_t m_expansionTargetID = 0;

	bool m_launchingAttack = false; // If an attack has been ordered on a star

	int m_attackTimer = 0; // Time to when the AI checks if the expansion target has been captured
	int m_attackFrustration = 0;

	int m_rallyTimer = 0;
};

class DefenseAI : public SubAI {
public:
	virtual void update(Faction& faction, Brain& brain, const AllianceList& alliances) override;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_fortifyingTimer;
	}
	
	int m_fortifyingTimer = 0;
};

class EconomyAI : public SubAI {
public:
	enum class EconomyState {
		DEVELOPING_PLANETS,
		BUILDING_SHIPS,
		NONE
	};
	
	virtual void update(Faction& faction, Brain& brain, const AllianceList& alliances) override;
	void handleColonies(Faction & faction);
	void handleShipDesigns(Faction & faction);
	void handleShips(Faction & faction);
	void handleStars(Faction & faction);
	void researchRandomTech(Faction& faction);
	bool researchStarterTechs(Faction& faction);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_state;
		archive & m_stateChangeTimer;
	}

	// Returns true if all wanted buildings were built
	bool buildColonyBuilding(Planet& planet, Faction& faction);

	EconomyState m_state = EconomyState::NONE;
	int m_stateChangeTimer = 0;
};

class Brain {
public:
	void onStart(Faction& faction);
	void onStarTakeover(Faction& faction, Star& star);
	void onResearchComplete(Faction& faction);
	void onColonization(Faction& faction, Planet& planet);
	void controlFaction(Faction& faction, const AllianceList& alliances);
	void controlSubAI(Faction& faction, SubAI& subAI, const AllianceList& alliances);
	void reinitAfterLoad(Constellation* constellation);

	MilitaryAI militaryAI;
	DefenseAI defenseAI;
	EconomyAI economyAI;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & militaryAI;
		archive & defenseAI;
		archive & economyAI;
		archive & m_personality.aggressiveness;
		archive & m_personality.economizer;
	}
	
	struct Personality {
		// All vals are 0-1
		float aggressiveness = 0.5f;
		float economizer = 0.5f;
	} m_personality;
};

