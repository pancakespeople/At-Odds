#pragma once
class Player {
public:
	int getFaction() { return m_factionID; }

	void setFaction(int allegiance) { m_factionID = allegiance; }

	bool hasFogOfWar() { return m_hasFogOfWar; }

	void enableFogOfWar() { m_hasFogOfWar = true; }

	void disableFogOfWar() { m_hasFogOfWar = false; }

private:
	bool m_hasFogOfWar = false;

	int m_factionID = -1;
};

