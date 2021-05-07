#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>

#include "SimpleIni.h"
#include "Building.h"

class Star;
class Spaceship;
class Constellation;
class GameState;
class MainMenu;
class Background;
class EffectsEmitter;

// This class handles GUI for selecting and moving units
class UnitGUI {
public:
	UnitGUI();
	
	void update(const sf::RenderWindow& window, Star* currentStar, int playerFaction);

	void draw(sf::RenderWindow& window);

	void onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state, std::vector<std::unique_ptr<Star>>& stars);
	
private:
	void drawStarPath(Star* begin, Star* end);

	sf::RectangleShape m_mouseSelectionBox;

	std::vector<Spaceship*> m_selectedShips;
};


class HelpWindow {
public:
	HelpWindow() {}

	void open(tgui::Gui& gui);
};

class BuildGUI {
public:
	struct BuildingSelector {
		BuildingPrototype prototype;
		tgui::Picture::Ptr icon;
		tgui::Panel::Ptr panel;
	};
	
	BuildGUI() {}

	void open(tgui::Gui& gui);

	void onBuildIconMouseEnter();

	void onBuildIconMouseExit();

	void onBuildIconClick(tgui::Gui& gui);

	void addBuildingSelector(Building::BUILDING_TYPE type);

	void onBuildingSelectorMouseEnter(int selectorIdx);
	
	void onBuildingSelectorMouseExit(int selectorIdx);

private:
	tgui::Picture::Ptr m_buildIcon;
	tgui::Panel::Ptr m_buildIconPanel;
	tgui::ScrollablePanel::Ptr m_buildPanel;
	std::vector<BuildingSelector> m_buildingSelectors;
};

class PlayerGUI {
public:
	PlayerGUI() {}

	void open(tgui::Gui& gui);

	BuildGUI& getBuildGUI() { return m_buildGUI; }

private:
	HelpWindow m_helpWindow;
	BuildGUI m_buildGUI;
};

class NewGameMenu {
public:
	NewGameMenu() {}

	void open(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu);

	void close();

	bool isOpen() { return m_window->isEnabled(); }

	PlayerGUI& getPlayerGUI() { return m_playerGui; }

private:
	void backToMainMenu(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu);

	void onStarsSliderChange(tgui::Gui& gui);

	void onFactionsSliderChange(tgui::Gui& gui);

	void startNewGame(tgui::Gui& gui, Constellation& constellation, GameState& state);

	tgui::ChildWindow::Ptr m_window;

	PlayerGUI m_playerGui;
};

class OptionsMenu {
public: 
	struct Settings {
		std::string resolution = "1366x768";
		float audioVolume = 100.0f;
		bool fullscreen = false;
	};

	OptionsMenu();

	void open(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu);

	void close(tgui::Gui& gui);

	void updateGameSettings(sf::RenderWindow& window, Background& background, tgui::Gui& gui, EffectsEmitter& emitter, bool force = false);

	sf::Vector2i getResolution();

	bool isOpen() { return m_window->isEnabled(); }

	Settings getSettings() { return m_settings; }

private:
	void backToMainMenu(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu);

	void onTabChange(tgui::Gui& gui);

	std::string getKeyIni(CSimpleIniA& ini, const char* section, const char* key, const char* defaultValue);

	void saveSettingsToFile();

	void changeSettings(tgui::Gui& gui);

	tgui::ChildWindow::Ptr m_window;

	tgui::Group::Ptr m_group;

	Settings m_settings;

	bool m_updateGameSettings = false;

	bool m_displayChanged = false;
};

class MainMenu {
public:
	MainMenu() {}

	void open(tgui::Gui& gui, Constellation& constellation, GameState& state);

	void close();

	void onEvent(sf::Event& ev, tgui::Gui& gui, Constellation& constellation, GameState& state);

	OptionsMenu& getOptionsMenu() { return m_optionsMenu; }

	NewGameMenu& getNewGameMenu() { return m_newGameMenu; }

private:
	void exitGame(GameState& state);

	void toNewGameMenu(tgui::Gui& gui, Constellation& constellation, GameState& state);

	void toOptionsMenu(tgui::Gui& gui, Constellation& constellation, GameState& state);

	tgui::ChildWindow::Ptr m_window;
	NewGameMenu m_newGameMenu;
	OptionsMenu m_optionsMenu;
	bool m_opened = false;
};

