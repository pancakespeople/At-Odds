#pragma once
#include "../GUI.h"

// This class handles GUI for selecting and moving units
class UnitGUI {
public:
	UnitGUI();

	void open(tgui::Gui& gui);
	void update(const sf::RenderWindow& window, Star* currentStar, int playerFaction, tgui::Panel::Ptr mainPanel);
	void draw(sf::RenderWindow& window);
	void onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state, std::vector<std::unique_ptr<Star>>& stars, tgui::Panel::Ptr mainPanel);

	std::vector<Spaceship*>& getSelectedShips() { return m_selectedShips; }

private:
	void drawStarPath(Star* begin, Star* end);

	sf::RectangleShape m_mouseSelectionBox;

	std::vector<Spaceship*> m_selectedShips;
	std::vector<Building*> m_selectedBuildings;

	tgui::Panel::Ptr m_panel = nullptr;
	tgui::Label::Ptr m_label = nullptr;
};