#pragma once
#include "../GUI.h"
#include "PlayerGUI.h"

class NewGameMenu {
public:
	NewGameMenu() {}

	void open(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu);
	void close();
	void addGameStartCallbacK(std::function<void()> func) { m_gameStartCallbacks.push_back(func); }

	bool isOpen() { return m_window->isEnabled(); }

	PlayerGUI& getPlayerGUI() { return m_playerGui; }

private:
	void onStarsSliderChange(tgui::Gui& gui);
	void onFactionsSliderChange(tgui::Gui& gui);
	void startNewGame(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu);

	GUI::ChildWindow::Ptr m_window;

	PlayerGUI m_playerGui;

	std::vector<std::function<void()>> m_gameStartCallbacks;
};