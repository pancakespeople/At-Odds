#pragma once
#include "../GUI.h"

class MinimapGUI {
public:
	MinimapGUI() {}

	void draw(sf::RenderWindow& window, Star* currentStar, int playerAllegiance);
	void update(const sf::RenderWindow& window, GameState& state);

private:
	sf::CircleShape m_minimapCircle;
	sf::View m_view;
};