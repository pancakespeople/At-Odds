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

	void open(tgui::Gui& gui, GameState& state);
	void onEvent(sf::Event& ev, tgui::Gui& gui, GameState& state);

	/*const sf::Clock& getUpdateClock() { return m_updateClock; }
	int getUpdatesPerSecondTarget() { return m_updatesPerSecondTarget; }
	void restartUpdateClock() { m_updateClock.restart(); }*/

private:
	tgui::Label::Ptr m_timescaleLabel;
	/*int m_timescale = 1;
	int m_updatesPerSecondTarget = 60;
	sf::Clock m_updateClock;*/
};

namespace GUI {

	struct Icon {
		void open(tgui::Gui& gui, tgui::Layout2d pos, tgui::Layout2d size, const std::string& picPath, const std::string& title = "");
		void draw(sf::RenderWindow& window);
		void setLengthScale(float scale) { m_lengthScale = scale; }

		tgui::Panel::Ptr panel;
	private:
		tgui::Theme m_hoverTheme;
		bool m_mouseHovered = false;
		float m_titleProgress = 0.0f;
		float m_lengthScale = 1.0f;
		sf::Text m_titleText;
	};

	struct Button : public tgui::Button {
		typedef std::shared_ptr<Button> Ptr;

		static Button::Ptr create(const std::string& text = "");
		void setClickSound(const std::string& filePath) { m_clickSoundPath = filePath; }

	private:
		std::string m_clickSoundPath = "data/sound/click.wav";
	};

	struct ChildWindow : public tgui::ChildWindow {
		typedef std::shared_ptr<ChildWindow> Ptr;

		static ChildWindow::Ptr create(const std::string& title = "", unsigned int titleButtons = TitleButton::Close);

	private:
		std::string m_closeSoundPath = "data/sound/scifiboop.wav";
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

namespace GUIUtil {
	void displayResourceCost(tgui::Group::Ptr group, const std::unordered_map<std::string, float>& totalResourceCost, int yPosPercent, int percentStep = 5);
}