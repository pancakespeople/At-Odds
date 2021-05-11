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
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& m_hasFogOfWar;
		archive& m_factionID;
		archive& m_color;
	}

	bool m_hasFogOfWar = false;
	int m_factionID = -1;
	
	sf::Color m_color;
};

