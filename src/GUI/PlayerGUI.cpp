#include "gamepch.h"
#include "PlayerGUI.h"
#include "../Constellation.h"

void PlayerGUI::open(tgui::Gui& gui, GameState& state, Constellation& constellation, bool spectator) {
	// An invisible bottom level panel to help with gui focusing
	mainPanel = tgui::Panel::create();
	mainPanel->getRenderer()->setOpacity(0.0f);
	gui.add(mainPanel);

	Faction* playerFaction = constellation.getFaction(state.getPlayer().getFaction());

	if (!spectator) {
#ifdef NDEBUG
		helpWindow.open(gui);
#endif
		buildGUI.open(gui, playerFaction);
		unitGUI.open(gui);
		planetGUI.open(gui, state, playerFaction);
		timescaleGUI.open(gui);
		resourceGUI.open(gui);
		shipDesignerGUI.open(gui, playerFaction);
		announcerGUI.open(gui);
		colonyListGUI.open(gui, state, constellation);
		techGUI.open(gui, playerFaction);
	}
	else {
		unitGUI.open(gui);
		planetGUI.open(gui, state, playerFaction);
		timescaleGUI.open(gui);
	}
}

void PlayerGUI::update(sf::RenderWindow& window, GameState& state, Constellation& constellation, tgui::Gui& gui) {
	Faction* playerFaction = constellation.getFaction(state.getPlayer().getFaction());

	unitGUI.update(window, state.getLocalViewStar(), state.getPlayer().getFaction(), mainPanel);
	planetGUI.update(state);
	resourceGUI.update(constellation, state.getPlayer());
	announcerGUI.update(gui, playerFaction);
	buildingGUI.update();
	techGUI.update(playerFaction);
}