#pragma once
#include "../GUI.h"

class MinimapGUI;

// This class handles GUI for selecting and moving units
class UnitGUI {
public:
	UnitGUI();

	void open(tgui::Gui& gui);
	void update(const sf::RenderWindow& window, Renderer& renderer, Star* currentStar, int playerFaction, tgui::Panel::Ptr mainPanel, MinimapGUI& minimap);
	void draw(sf::RenderWindow& window);
	void onEvent(const sf::Event& ev, sf::RenderWindow& window, Renderer& renderer, GameState& state, Constellation& constellation, tgui::Panel::Ptr mainPanel, MinimapGUI& minimap);
	void setDisabled(bool disabled) { m_disabled = disabled; }

	bool isSelecting() const { return m_selecting; }

	std::vector<Spaceship*>& getSelectedShips() { return m_selectedShips; }

private:
	void drawStarPath(Star* begin, Star* end);
	void onMouseClick(const sf::Event& ev, const Renderer& renderer, Star* currentStar, int playerFaction);
	void onSelect(const Renderer& renderer, Star* star, int playerAllegiance);
	
	sf::Vector2f getAveragePosOfSelectedShips();

	sf::RectangleShape m_mouseSelectionBox;

	std::vector<Spaceship*> m_selectedShips;
	std::vector<Building*> m_selectedBuildings;

	tgui::Panel::Ptr m_panel = nullptr;
	tgui::Label::Ptr m_label = nullptr;

	bool m_selecting = false;
	bool m_mouseDown = false;
	bool m_disabled = false;
};