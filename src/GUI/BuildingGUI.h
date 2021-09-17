#pragma once
#include "../GUI.h"

class Building;

// Not to be confused with BuildGUI, this one is for the popup when you click on a building
class BuildingGUI {
public:
	BuildingGUI() {}

	void open() { m_openBuilding = nullptr; }
	void onEvent(const sf::Event& ev, const sf::RenderWindow& window, tgui::Gui& gui, GameState& state, Constellation& constellation, tgui::Panel::Ptr mainPanel);
	void update();

private:
	tgui::ChildWindow::Ptr m_window;
	sf::Vector2f m_lastMouseDownPos;
	Building* m_openBuilding = nullptr;
};