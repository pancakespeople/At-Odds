#pragma once
class Faction;
class Star;

class Brain {
public:
	void onStart(Faction* faction);

	void onSpawn(Faction* faction);

	void controlFaction(Faction* faction);

private:
	Star* m_expansionTarget = nullptr; // Star that the AI wants to attack
	
	bool m_launchingAttack = false; // If an attack has been ordered on a star

	int m_attackTimer = 0; // Time to when the AI checks if the expansion target has been captured

	int m_attackFrustration = 0;
};

