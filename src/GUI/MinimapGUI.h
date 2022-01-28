#pragma once
#include "../GUI.h"

class UnitGUI;
class AllianceList;

class MinimapGUI {
public:
	MinimapGUI() {}

	void draw(sf::RenderWindow& window, Star* currentStar, int playerAllegiance, Camera& camera, const AllianceList& alliances);
	void update(const sf::RenderWindow& window, GameState& state, const UnitGUI& unitGUI);

	bool isMouseInMinimap(const sf::RenderWindow& window);

	sf::FloatRect getViewport() { return m_view.getViewport(); }

private:
	sf::CircleShape m_minimapCircle;
	sf::View m_view;
};