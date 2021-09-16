#pragma once
#include "../GUI.h"
#include "ShipDesigner.h"
#include "PlanetGUI.h"
#include "UnitGUI.h"

struct PlayerGUI {
	PlayerGUI() {}

	void open(tgui::Gui& gui, GameState& state, Constellation& constellation, bool spectator);

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
};