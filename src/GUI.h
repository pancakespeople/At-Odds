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

class ColonyListGUI {
public:
	ColonyListGUI() {}

	void open(tgui::Gui& gui, GameState& state, Constellation& constellation);

private:
	GameWidget::Icon m_icon;
	tgui::ChildWindow::Ptr m_window = nullptr;
};

namespace GUIUtil {
	void displayResourceCost(tgui::Group::Ptr group, const std::unordered_map<std::string, float>& totalResourceCost, int yPosPercent, int percentStep = 5);
}