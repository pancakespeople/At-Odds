#pragma once
#include "../GUI.h"


class PlanetGUI {
public:
	PlanetGUI() {}

	void open(tgui::Gui& gui, GameState& state, Faction* playerFaction, const Constellation& constellation);
	void updateSync(GameState& state);
	void update();
	void draw(Renderer& renderer, const sf::RenderWindow& window);
	void onEvent(const sf::Event& ev, tgui::Gui& gui, GameState& state, Faction* playerFaction, const sf::RenderWindow& window, Renderer& renderer, Star* currentStar, tgui::Panel::Ptr mainPanel, const Constellation& constellation);

private:
	void setSelectedPlanet(tgui::ComboBox::Ptr planetList, GameState& state, Faction* playerFaction, tgui::Gui& gui, int index, const Constellation& constellation);
	void switchSideWindow(const std::string& name, tgui::Gui& gui);
	void createBuildingsButton(tgui::Gui& gui, Planet& planet, Faction* playerFaction);
	void createLawsButton(tgui::Gui& gui, GameState& state, Planet& planet);
	void createColonyAndResourcesButtons(tgui::Gui& gui, GameState& state, Planet& planet, Faction* playerFaction, const Constellation& constellation);
	void displayBuildingInfo(ColonyBuilding& building, Planet& planet, bool buildInfo);
	void createBuildStatusLabel(Planet& planet, const ColonyBuilding& building);
	void createEventsButton(tgui::Gui& gui, const Planet& planet);
	void createTradeButton(tgui::Gui& gui, Planet& planet);
	void createMapButton(tgui::Gui& gui);
	void openBuildingsPanel(tgui::Gui& gui, Planet& planet, Faction* playerFaction);
	void closePanel(tgui::Gui& gui);
	void updateTileInfo(sf::Vector2i tile);
	static void updateTrendWidget(tgui::Label::Ptr& label, float trend);

	GUI::Icon m_planetIcon;
	tgui::Panel::Ptr m_planetInfoPanel;
	tgui::Panel::Ptr m_buttonPanel;
	tgui::VerticalLayout::Ptr m_buttonPanelLayout;
	tgui::ChildWindow::Ptr m_planetPanel;
	tgui::ChildWindow::Ptr m_sideWindow;
	tgui::Canvas::Ptr m_planetMapCanvas;
	std::function<void(Planet&)> m_updateFunction = nullptr;
	tgui::Panel::Ptr m_mapInfoPanel;

	bool m_showPopulation = false;
	sf::Vector2i m_selectedTile = { -1, -1 };

	Planet* m_currentPlanet = nullptr;
};