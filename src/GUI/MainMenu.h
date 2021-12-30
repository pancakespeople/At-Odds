#pragma once
#include "../GUI.h"
#include "NewGameMenu.h"
#include "OptionsMenu.h"

struct PlayerGUI;

class MainMenu {
public:
	MainMenu() {}

	void open(tgui::Gui& gui, Constellation& constellation, GameState& state);
	void close(tgui::Gui& gui);
	void onEvent(sf::Event& ev, tgui::Gui& gui, Constellation& constellation, GameState& state);
	void drawPreview(Renderer& renderer, const GameState& state, float time);
	void spawnArena(tgui::Gui& gui, Constellation& constellation, GameState& state, PlayerGUI& playerGUI);
	void updateArena(int ticks, Constellation& constellation);
	void setForceOpen(bool forceOpen) { m_forceOpen = forceOpen; }
	OptionsMenu& getOptionsMenu() { return m_optionsMenu; }
	NewGameMenu& getNewGameMenu() { return m_newGameMenu; }

	bool isForceOpen() { return m_forceOpen; }

private:
	void exitGame(GameState& state);

	tgui::Panel::Ptr m_panel;
	tgui::Label::Ptr m_title;
	NewGameMenu m_newGameMenu;
	OptionsMenu m_optionsMenu;
	bool m_opened = false;
	bool m_forceOpen = true;
	float m_starSeed = 0.0f;
	sf::RectangleShape m_starRect;
	sf::RectangleShape m_planetRect;
};