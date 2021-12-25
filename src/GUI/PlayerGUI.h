#pragma once
#include "../GUI.h"
#include "ShipDesigner.h"
#include "PlanetGUI.h"
#include "UnitGUI.h"
#include "BuildingGUI.h"
#include "TechGUI.h"
#include "ResourceGUI.h"
#include "BuildGUI.h"
#include "MinimapGUI.h"
#include "NewsGUI.h"
#include "ColonyListGUI.h"

enum class PlayerGUIState {
	CLOSED,
	PLAYER,
	SPECTATOR,
};

struct PlayerGUI {
	PlayerGUI() {}

	void open(tgui::Gui& gui, GameState& state, Constellation& constellation, PlayerGUIState guiState);
	void update(sf::RenderWindow& window, GameState& state, Constellation& constellation, tgui::Gui& gui);
	void onEvent(const sf::Event& ev, tgui::Gui& gui);
	void setVisible(tgui::Gui& gui, bool visible);
	void draw(sf::RenderWindow& window, GameState& state, Constellation& constellation, Player& player);
	
	// This update function is to be called in sync with game time (wont be called when game is paused)
	void updateSync(sf::RenderWindow& window, GameState& state, Constellation& constellation, tgui::Gui& gui);

	tgui::Panel::Ptr mainPanel;

	HelpWindow helpWindow;
	BuildGUI buildGUI;
	UnitGUI unitGUI;
	PlanetGUI planetGUI;
	BuildingGUI buildingGUI;
	TimescaleGUI timescaleGUI;
	ResourceGUI resourceGUI;
	ShipDesignerGUI shipDesignerGUI;
	AnnouncerGUI announcerGUI;
	MinimapGUI minimapGUI;
	ColonyListGUI colonyListGUI;
	TechGUI techGUI;
	NewsGUI newsGUI;

private:
	bool m_visible = true;
	PlayerGUIState m_state = PlayerGUIState::CLOSED;
};