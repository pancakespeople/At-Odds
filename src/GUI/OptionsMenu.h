#pragma once
#include "../GUI.h"

class MusicPlayer;

class OptionsMenu {
public:
	struct Settings {
		std::string resolution = "1366x768";
		float audioVolume = 100.0f;
		float musicVolume = 100.0f;
		bool fullscreen = false;
	};

	OptionsMenu();

	void open(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu);
	void close(tgui::Gui& gui);
	void updateGameSettings(sf::RenderWindow& window, Background& background, tgui::Gui& gui, EffectsEmitter& emitter, MusicPlayer& musicPlayer, Camera& camera, bool force = false);

	sf::Vector2i getResolution();

	bool isOpen() { return m_window->isEnabled(); }

	Settings getSettings() { return m_settings; }

private:
	void onTabChange(tgui::Gui& gui);
	void saveSettingsToFile();
	void changeSettings(tgui::Gui& gui);

	tgui::ChildWindow::Ptr m_window;
	tgui::Group::Ptr m_group;

	Settings m_settings;

	bool m_updateGameSettings = false;
	bool m_displayChanged = false;
};