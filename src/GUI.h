#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>

#include "SimpleIni.h"
#include "Building.h"
#include "GameState.h"
#include "Planet.h"

class Star;
class Spaceship;
class Constellation;
class GameState;
class MainMenu;
class Background;
class EffectsEmitter;
class Player;
class SaveLoader;
class Planet;
class Background;

// This class handles GUI for selecting and moving units
class UnitGUI {
public:
	UnitGUI();

	void open(tgui::Gui& gui);
	void update(const sf::RenderWindow& window, Star* currentStar, int playerFaction, tgui::Panel::Ptr mainPanel);
	void draw(sf::RenderWindow& window);
	void onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state, std::vector<std::unique_ptr<Star>>& stars);
	
private:
	void drawStarPath(Star* begin, Star* end);

	sf::RectangleShape m_mouseSelectionBox;

	std::vector<Spaceship*> m_selectedShips;

	tgui::Panel::Ptr m_panel = nullptr;
	tgui::Label::Ptr m_label = nullptr;
};


class HelpWindow {
public:
	HelpWindow() {}
	void open(tgui::Gui& gui);
	void close();

private:
	tgui::ChildWindow::Ptr m_window = nullptr;
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
	void draw(sf::RenderWindow& window, const Star* currentStar, const Player& player);
	void onEvent(const sf::Event& ev, const sf::RenderWindow& window, Star* currentLocalStar, const Player& player);

private:
	void onBuildIconMouseEnter();
	void onBuildIconMouseExit();
	void onBuildIconClick(tgui::Gui& gui);
	void addBuildingSelector(Building::BUILDING_TYPE type, const std::string& name);
	void onBuildingSelectorMouseEnter(int selectorIdx);
	void onBuildingSelectorMouseExit(int selectorIdx);
	void onBuildingSelectorClick(int selectorIdx);

	tgui::Picture::Ptr m_buildIcon;
	tgui::Panel::Ptr m_buildIconPanel;
	tgui::Panel::Ptr m_buildPanel;
	std::vector<BuildingSelector> m_buildingSelectors;

	int m_selectedBuildingIdx = -1;
	bool m_canReceiveEvents = true;
};

class PlanetGUI {
public:
	PlanetGUI() {}

	void open(tgui::Gui& gui, GameState& state);
	void update(GameState& state);
	void onEvent(const sf::Event& ev, tgui::Gui& gui, GameState& state, const sf::RenderWindow& window, Star* currentStar, tgui::Panel::Ptr mainPanel);

private:
	void setSelectedPlanet(tgui::ComboBox::Ptr planetList, GameState& state, tgui::Gui& gui, int index);
	void switchSideWindow(const std::string& name, tgui::Gui& gui);
	
	tgui::Panel::Ptr m_planetIconPanel;
	tgui::Panel::Ptr m_planetPanel;
	tgui::Panel::Ptr m_planetInfoPanel;
	tgui::ChildWindow::Ptr m_sideWindow;
};

// Not to be confused with BuildGUI, this one is for the popup when you click on a building
class BuildingGUI {
public:
	BuildingGUI() {}

	void onEvent(const sf::Event& ev, const sf::RenderWindow& window, tgui::Gui& gui, GameState& state, Constellation& constellation, tgui::Panel::Ptr mainPanel);

private:
	tgui::ChildWindow::Ptr m_window;
};

class TimescaleGUI {
public:
	TimescaleGUI() {}

	void open(tgui::Gui& gui);
	void onEvent(sf::Event& ev, tgui::Gui& gui, int& updatesPerSecondTarget);

private:
	tgui::Label::Ptr m_timescaleLabel;
	int m_timescale = 1;
};

class ResourceGUI {
public:
	void open(tgui::Gui& gui);
	void update(Constellation& constellation, Player& player);

private:
	tgui::Group::Ptr m_resourceGroup;
};

namespace GameWidget {

	struct Icon {
		void open(tgui::Gui& gui, tgui::Layout2d pos, tgui::Layout2d size, const std::string& picPath);

		tgui::Panel::Ptr panel;
	};

}

class ShipDesignerGUI {
public:
	ShipDesignerGUI() {}

	void open(tgui::Gui& gui, Faction* playerFaction);
	void displayShipInfo(Faction* playerFaction);
	bool canChassisFitWeapons(Faction* playerFaction);
	void displayShipDesigns(Faction* playerFaction);
	static void displayShipResourceCost(tgui::Group::Ptr group, const std::unordered_map<PlanetResource::RESOURCE_TYPE, float>& totalResourceCost, int yPosPercent);

private:
	GameWidget::Icon m_icon;
	tgui::ChildWindow::Ptr m_window;
};

class AnnouncerGUI {
public:
	AnnouncerGUI() {}

	void open(tgui::Gui& gui);
	void update(tgui::Gui& gui, Faction* playerFaction);

private:
	tgui::Label::Ptr m_label;
};

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
};

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
	void startNewGame(tgui::Gui& gui, Constellation& constellation, GameState& state);

	tgui::ChildWindow::Ptr m_window;

	PlayerGUI m_playerGui;

	std::vector<std::function<void()>> m_gameStartCallbacks;
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
	void onTabChange(tgui::Gui& gui);
	void saveSettingsToFile();
	void changeSettings(tgui::Gui& gui);

	std::string getKeyIni(CSimpleIniA& ini, const char* section, const char* key, const char* defaultValue);

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

class DebugConsole {
public:
	struct Command {
		std::string command;
		std::vector<std::string> args;
	};
	
	DebugConsole() {}

	void open(tgui::Gui& gui);
	void close(tgui::Gui& gui);
	void onEvent(sf::Event& ev, tgui::Gui& gui, GameState& state);
	void processCommand(std::string rawCommand);
	void runCommands(Constellation& constellation, GameState& state, sf::RenderWindow& window, tgui::Gui& gui, PlayerGUI& playerGUI);

	bool isOpen() { return m_console != nullptr; }
	bool validateArgs(const Command& command, int numArgs);
	bool validateState(const Command& command, const GameState& state, GameState::State requestedState);
	bool validateNotState(const Command& command, const GameState& state, GameState::State notState);

private:
	tgui::Group::Ptr m_console;
	tgui::ChatBox::Ptr m_chatBox;
	tgui::EditBox::Ptr m_editBox;

	std::queue<Command> m_commandQueue;
};
