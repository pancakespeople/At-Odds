#pragma once

class Spaceship;
class Camera;
class GameState;

class Player {
public:
	int getFaction() const { return m_factionID; }

	void setFaction(int allegiance, sf::Color color);
	void enableFogOfWar() { m_hasFogOfWar = true; }
	void disableFogOfWar() { m_hasFogOfWar = false; }
	void setControlledShip(Spaceship* ship);
	void update(GameState& state);
	void onEvent(const sf::Event& ev);

	bool hasFogOfWar() const { return m_hasFogOfWar; }

	sf::Color getColor() const { return m_color; }

	Spaceship* getControlledShip() { return m_controlledShip; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& m_hasFogOfWar;
		archive& m_factionID;
		archive& m_color;

		if (m_controlledShip != nullptr) {
			m_controlledShip->setAIEnabled(true);
		}
	}

	bool m_hasFogOfWar = false;
	int m_factionID = -1;
	
	sf::Color m_color;

	Spaceship* m_controlledShip = nullptr;
};

