#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>

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
class UnitGUI;

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

	void open(tgui::Gui& gui, Faction* playerFaction);
	void draw(sf::RenderWindow& window, Star* currentStar, const Player& player);
	void onEvent(const sf::Event& ev, const sf::RenderWindow& window, Star* currentLocalStar, Faction* playerFaction, UnitGUI& unitGUI, tgui::Panel::Ptr mainPanel);

private:
	void onBuildIconMouseEnter();
	void onBuildIconMouseExit();
	void onBuildIconClick(tgui::Gui& gui, Faction* playerFaction);
	void addBuildingSelector(const std::string& type);
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

class TimescaleGUI {
public:
	TimescaleGUI() {}

	void open(tgui::Gui& gui);
	void onEvent(sf::Event& ev, tgui::Gui& gui);

	const sf::Clock& getUpdateClock() { return m_updateClock; }
	int getUpdatesPerSecondTarget() { return m_updatesPerSecondTarget; }
	void restartUpdateClock() { m_updateClock.restart(); }

private:
	tgui::Label::Ptr m_timescaleLabel;
	int m_timescale = 1;
	int m_updatesPerSecondTarget = 60;
	sf::Clock m_updateClock;
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

class AnnouncerGUI {
public:
	AnnouncerGUI() {}

	void open(tgui::Gui& gui);
	void update(tgui::Gui& gui, Faction* playerFaction);

private:
	tgui::Label::Ptr m_label;
};

class MinimapGUI {
public:
	MinimapGUI() {}

	void draw(sf::RenderWindow& window, Star* currentStar, int playerAllegiance);
};

class ColonyListGUI {
public:
	ColonyListGUI() {}

	void open(tgui::Gui& gui, GameState& state, Constellation& constellation);

private:
	GameWidget::Icon m_icon;
	tgui::ChildWindow::Ptr m_window = nullptr;
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

	tgui::ChildWindow::Ptr m_window;
	tgui::Group::Ptr m_group;

	Settings m_settings;

	bool m_updateGameSettings = false;
	bool m_displayChanged = false;
};

namespace GUIUtil {
	void displayResourceCost(tgui::Group::Ptr group, const std::unordered_map<std::string, float>& totalResourceCost, int yPosPercent, int percentStep = 5);
}