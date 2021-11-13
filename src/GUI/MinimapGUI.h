#pragma once
#include "../GUI.h"

class UnitGUI;

class MinimapGUI {
public:
	MinimapGUI() {}

	void draw(sf::RenderWindow& window, Star* currentStar, int playerAllegiance, Camera& camera);
	void update(const sf::RenderWindow& window, GameState& state, const UnitGUI& unitGUI);

	bool isMouseInMinimap(const sf::RenderWindow& window);

private:
	sf::CircleShape m_minimapCircle;
	sf::View m_view;
};