#include "gamepch.h"
#include "GUI.h"
#include "Spaceship.h"
#include "Star.h"
#include "Order.h"
#include "Pathfinder.h"
#include "Hyperlane.h"
#include "Constellation.h"
#include "GameState.h"
#include "Background.h"
#include "EffectsEmitter.h"
#include "Sounds.h"
#include "TextureCache.h"
#include "SaveLoader.h"
#include "Math.h"
#include "ext/toml.hpp"
#include "TOMLCache.h"
#include "JumpPoint.h"
#include "GUI/MainMenu.h"
#include "GUI/UnitGUI.h"

void HelpWindow::open(tgui::Gui& gui) {
	auto window = tgui::ChildWindow::create("Help");
	window->setSize("33%", "66%");
	window->setPosition("(parent.size - size) / 2");
	window->setInheritedOpacity(0.75f);
	gui.add(window);

	m_window = window;

	std::string helpText = "Welcome to At Odds.\n"
		"Controls:\n"
		"WASD, arrow keys or middle mouse - move camera\n"
		"Tab - Switch to constellation view\n"
		"Esc - Main menu\n"
		"Click drag - select units\n"
		"Right click - order units\n"
		"\n"
		"How to play:\n"
		"Colonize other planets by going to its laws tab and setting colonization to be legal, your people will colonize the planet automatically.\n"
		"Build more ships by clicking on your ship factory and changing its settings.\n"
		"Your goal is to eliminate all your enemies. Good luck!"
		;

	auto text = tgui::Label::create(helpText);
	text->setSize("100%", "100%");
	window->add(text);
}

void HelpWindow::close() {
	if (m_window != nullptr) {
		m_window->close();
	}
}

void TimescaleGUI::open(tgui::Gui& gui) {
	m_timescaleLabel = tgui::Label::create();
	m_timescaleLabel->setOrigin(0.5f, 0.5f);
	m_timescaleLabel->setPosition("50%", "10%");
	m_timescaleLabel->setTextSize(25);
	m_timescaleLabel->setVisible(false);
	gui.add(m_timescaleLabel);
}

void TimescaleGUI::onEvent(sf::Event& ev, tgui::Gui& gui) {
	if (m_timescaleLabel != nullptr) {
		tgui::Widget::Ptr focused = gui.getFocusedLeaf();
		tgui::String focusedType;
		if (focused != nullptr) {
			focusedType = focused->getWidgetType();
		}

		if (focusedType != "EditBox") {
			if (ev.type == sf::Event::KeyReleased && ((ev.key.code == sf::Keyboard::Equal && ev.key.shift) ||
				(ev.key.code == sf::Keyboard::Dash))) {
				// + or - pressed

				gui.remove(m_timescaleLabel);
				open(gui);

				if (ev.key.code == sf::Keyboard::Dash) {
					if (m_timescale > 1) {
						m_timescale = m_timescale >> 1;
					}
				}
				else {
					if (m_timescale < 64) {
						m_timescale = m_timescale << 1;
					}
				}

				m_updatesPerSecondTarget = 60 * m_timescale;

				m_timescaleLabel->setText("Timescale: " + std::to_string(m_timescale) + "x");
				m_timescaleLabel->setVisible(true);
				m_timescaleLabel->hideWithEffect(tgui::ShowAnimationType::Fade, tgui::Duration(4000));
			}
			else if (ev.type == sf::Event::KeyReleased && ev.key.code == sf::Keyboard::Space) {
				gui.remove(m_timescaleLabel);
				open(gui);

				if (m_timescale != 0) {
					m_timescale = 0;

					m_timescaleLabel->setText("Paused");
					m_timescaleLabel->setVisible(true);
				}
				else {
					m_timescale = 1;

					m_updateClock.restart();

					m_timescaleLabel->setText("Unpaused");
					m_timescaleLabel->setVisible(true);
					m_timescaleLabel->hideWithEffect(tgui::ShowAnimationType::Fade, tgui::Duration(4000));
				}

				m_updatesPerSecondTarget = 60 * m_timescale;
			}
		}
	}
}

void GameWidget::Icon::open(tgui::Gui& gui, tgui::Layout2d pos, tgui::Layout2d size, const std::string& picPath) {
	panel = tgui::Panel::create();
	panel->setPosition(pos);
	panel->setSize(size);
	panel->getRenderer()->setOpacity(0.75f);

	panel->onMouseEnter([this]() {
		panel->getRenderer()->setBackgroundColor(tgui::Color::White);
		panel->setRenderer(tgui::Theme().getRenderer("Panel"));
	});

	panel->onMouseLeave([this]() {
		panel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
		panel->getRenderer()->setOpacity(0.75f);
		panel->setRenderer(tgui::Theme::getDefault()->getRenderer("Panel"));
	});

	gui.add(panel);

	auto picture = tgui::Picture::create(picPath.c_str());
	picture->setSize("100%", "100%");
	panel->add(picture);
}

void AnnouncerGUI::open(tgui::Gui& gui) {
	m_label = tgui::Label::create();
	m_label->setOrigin(0.5f, 0.5f);
	m_label->setPosition("50%", "25%");
	m_label->setTextSize(25);
	m_label->setVisible(false);
	gui.add(m_label);
}

void AnnouncerGUI::update(tgui::Gui& gui, Faction* playerFaction) {
	if (playerFaction != nullptr) {
		std::deque<std::string>& announcements = playerFaction->getAnnouncementEvents();
		if (announcements.size() > 0) {
			gui.remove(m_label);
			open(gui);

			m_label->setText(announcements.front());
			announcements.pop_front();
			
			m_label->setVisible(true);
			m_label->hideWithEffect(tgui::ShowAnimationType::Fade, tgui::Duration(4000));
		}
	}
}

void GUIUtil::displayResourceCost(tgui::Group::Ptr group, const std::unordered_map<std::string, float>& totalResourceCost, int yPosPercent, int percentStep) {
	auto resourcesLabel = tgui::Label::create("Resources: ");
	resourcesLabel->setPosition("0%", (std::to_string(yPosPercent) + "%").c_str());
	group->add(resourcesLabel);

	yPosPercent += percentStep;

	// Add resource cost labels
	for (auto& resource : totalResourceCost) {
		Resource r;
		r.type = resource.first;

		std::stringstream labelString;
		labelString << r.getName() << ": " << std::fixed << std::setprecision(1) << resource.second;

		auto label = tgui::Label::create(labelString.str());
		label->setPosition("0%", (std::to_string(yPosPercent) + "%").c_str());
		group->add(label);

		yPosPercent += percentStep;
	}
}