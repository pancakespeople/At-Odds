#pragma once
#include "../GUI.h"

class MinimapGUI;
class Asteroid;

// This class handles GUI for selecting and moving units
class UnitGUI {
public:
	UnitGUI();

	void open(tgui::Gui& gui, Player& player);
	void update(const sf::RenderWindow& window, Renderer& renderer, Star* currentStar, Player& player, tgui::Panel::Ptr mainPanel, MinimapGUI& minimap);
	void updatePanel();
	void draw(sf::RenderWindow& window);
	void onEvent(const sf::Event& ev, sf::RenderWindow& window, Renderer& renderer, GameState& state, Constellation& constellation, tgui::Panel::Ptr mainPanel, MinimapGUI& minimap);
	void onRightClickWorldView(sf::RenderWindow & window, Renderer & renderer, Constellation & constellation);
	void onRightClickLocalView(sf::RenderWindow & window, Renderer & renderer, Constellation & constellation, GameState & state);
	void setDisabled(bool disabled) { m_disabled = disabled; }
	void cleanUpDeadStuff();
	void deselectAll();

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
	Asteroid* m_selectedAsteroid = nullptr;

	tgui::Panel::Ptr m_panel = nullptr;
	tgui::Label::Ptr m_label = nullptr;
	GUI::Button::Ptr m_possessButton = nullptr;

	bool m_selecting = false;
	bool m_mouseDown = false;
	bool m_disabled = false;
};