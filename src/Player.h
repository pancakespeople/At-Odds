#pragma once
class Player {
public:
	int getFaction() const { return m_factionID; }

	void setFaction(int allegiance, sf::Color color);

	bool hasFogOfWar() const { return m_hasFogOfWar; }

	void enableFogOfWar() { m_hasFogOfWar = true; }

	void disableFogOfWar() { m_hasFogOfWar = false; }

	sf::Color getColor() const { return m_color; }

private:
	bool m_hasFogOfWar = false;
	int m_factionID = -1;
	
	sf::Color m_color;
};

