#pragma once
#include "../GUI.h"

class MinimapGUI;

// This class handles GUI for selecting and moving units
class UnitGUI {
public:
	UnitGUI();

	void open(tgui::Gui& gui);
	void update(const sf::RenderWindow& window, Star* currentStar, int playerFaction, tgui::Panel::Ptr mainPanel, MinimapGUI& minimap);
	void draw(sf::RenderWindow& window);
	void onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state, std::vector<std::unique_ptr<Star>>& stars, tgui::Panel::Ptr mainPanel);

	bool isSelecting() const { return m_selecting; }

	std::vector<Spaceship*>& getSelectedShips() { return m_selectedShips; }

private:
	void drawStarPath(Star* begin, Star* end);
	sf::Vector2f getAveragePosOfSelectedShips();

	sf::RectangleShape m_mouseSelectionBox;

	std::vector<Spaceship*> m_selectedShips;
	std::vector<Building*> m_selectedBuildings;

	tgui::Panel::Ptr m_panel = nullptr;
	tgui::Label::Ptr m_label = nullptr;

	bool m_selecting = false;
};