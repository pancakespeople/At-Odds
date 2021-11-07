#pragma once
#include "../GUI.h"
#include "NewGameMenu.h"
#include "OptionsMenu.h"

class MainMenu {
public:
	MainMenu() {}

	void open(tgui::Gui& gui, Constellation& constellation, GameState& state);
	void close(tgui::Gui& gui);
	void onEvent(sf::Event& ev, tgui::Gui& gui, Constellation& constellation, GameState& state);
	OptionsMenu& getOptionsMenu() { return m_optionsMenu; }
	NewGameMenu& getNewGameMenu() { return m_newGameMenu; }

private:
	void exitGame(GameState& state);

	tgui::Panel::Ptr m_panel;
	tgui::Label::Ptr m_title;
	NewGameMenu m_newGameMenu;
	OptionsMenu m_optionsMenu;
	bool m_opened = false;
};