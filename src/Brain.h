#pragma once
class Faction;
class Star;

class Brain {
public:
	enum class AI_STATE {
		NONE,
		ATTACKING,
		FORTIFYING,
		SHIP_BUILDING
	};
	
	void onStart(Faction* faction);
	void onSpawn(Faction* faction);
	void controlFaction(Faction* faction);
	void considerAttack(Faction* faction);
	void considerFortifying(Faction* faction);
	void considerShipBuilding(Faction* faction);
	void considerChangingState();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_state;
		archive & m_stateChangeTimer;
		archive & m_personality.aggressiveness;
		archive & m_personality.admiral;
		archive & m_attackVars.expansionTarget;
		archive & m_attackVars.launchingAttack;
		archive & m_attackVars.attackTimer;
		archive & m_attackVars.attackFrustration;
		archive & m_fortifyingVars.fortifyingTimer;
	}
	
	AI_STATE m_state = AI_STATE::NONE;
	int m_stateChangeTimer = 0;

	struct Personality {
		// All vals are percent
		float aggressiveness = 0.5f;
		float admiral = 0.5f;
	} m_personality;

	struct AttackVars {
		Star* expansionTarget = nullptr; // Star that the AI wants to attack

		bool launchingAttack = false; // If an attack has been ordered on a star

		int attackTimer = 0; // Time to when the AI checks if the expansion target has been captured
		int attackFrustration = 0;
	} m_attackVars;

	struct FortifyingVars {
		int fortifyingTimer = 0;
	} m_fortifyingVars;
};

