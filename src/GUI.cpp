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
#include "toml.hpp"
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
	});

	panel->onMouseLeave([this]() {
		panel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
		panel->getRenderer()->setOpacity(0.75f);
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

void MinimapGUI::draw(sf::RenderWindow& window, Star* currentStar, int playerAllegiance) {
	if (currentStar != nullptr) {
		sf::View oldView = window.getView();
		float ratio = oldView.getSize().x / oldView.getSize().y;

		sf::View view;
		view.setCenter(currentStar->getPos());
		view.setSize(50000.0f * ratio, 50000.0f);
		view.setViewport(sf::FloatRect(0.8f, 0.75f, 0.25f, 0.25f));

		window.setView(view);

		sf::CircleShape dot;
		dot.setFillColor(sf::Color(125, 125, 125, 125));
		dot.setPosition(currentStar->getPos());
		dot.setRadius(25000.0f);
		dot.setOrigin(25000.0f, 25000.0f);
		window.draw(dot);

		dot.setFillColor(sf::Color::Yellow);
		dot.setPosition(currentStar->getPos());
		dot.setRadius(500.0f);
		dot.setOrigin(500.0f, 500.0f);

		window.draw(dot);

		if ((currentStar->isDrawingHidden() && currentStar->isDiscovered(playerAllegiance)) || playerAllegiance == -1) {
			for (auto& ship : currentStar->getSpaceships()) {
				dot.setPosition(ship->getPos());
				if (playerAllegiance == -1) {
					dot.setFillColor(ship->getFactionColor());
				}
				else {
					if (ship->getAllegiance() == playerAllegiance) {
						dot.setFillColor(sf::Color::Blue);
					}
					else {
						dot.setFillColor(sf::Color::Red);
					}
				}

				window.draw(dot);
			}
		}

		for (auto& jumpPoint : currentStar->getJumpPoints()) {
			dot.setPosition(jumpPoint.getPos());
			dot.setFillColor(sf::Color(128, 0, 128));
			window.draw(dot);
		}

		window.setView(oldView);
	}
}

void ColonyListGUI::open(tgui::Gui& gui, GameState& state, Constellation& constellation) {
	m_icon.open(gui, tgui::Layout2d("0%", "75%"), tgui::Layout2d("2.5%", "5%"), "data/art/colonyicon.png");
	
	m_icon.panel->onClick([this, &gui, &state, &constellation]() {
		if (m_window == nullptr) {
			m_window = tgui::ChildWindow::create("Colony List");
			m_window->setSize("50%", "50%");
			m_window->setOrigin(0.5, 0.5);
			m_window->setPosition(gui.getTarget()->getSize().x / 2.0, gui.getTarget()->getSize().y / 2.0);
			m_window->getRenderer()->setOpacity(0.75f);
			m_window->onClose([this]() {
				m_window = nullptr;
			});
			gui.add(m_window);

			auto listView = tgui::ListView::create();
			listView->setSize("100%", "100%");
			listView->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
			listView->addColumn("Type    ");
			listView->addColumn("Name");
			listView->addColumn("Population");
			listView->addColumn("Growth Rate");
			listView->addColumn("Temperature");
			listView->addColumn("Water");
			listView->addColumn("Atmosphere");
			listView->addColumn("Habitability");

			int playerFaction = state.getPlayer().getFaction();
			for (auto& star : constellation.getStars()) {
				int maxNameLength = (star->getName().length() + 5) * 10;
				listView->setColumnWidth(1, maxNameLength);
				int pos = 1;
				for (Planet& planet : star->getPlanets()) {
					if (planet.getColony().getAllegiance() == playerFaction) {
						std::vector<tgui::String> info;
						info.push_back(planet.getTypeString());
						info.push_back(planet.getName(star.get(), pos));
						info.push_back(std::to_string(planet.getColony().getPopulation()));
						info.push_back(std::to_string(planet.getColony().getGrowthRate(planet.getHabitability()) * 100.0f) + "%");
						info.push_back(std::to_string(planet.getTemperature()));
						info.push_back(std::to_string(planet.getWater()));
						info.push_back(std::to_string(planet.getAtmosphericPressure()));
						info.push_back(std::to_string(planet.getHabitability()));
						listView->addItem(info);
						listView->setItemData(listView->getItemCount() - 1, std::pair<int, int>(star->getID(), planet.getID()));
					}
					pos++;
				}
			}

			listView->onHeaderClick([listView](int index) {
				listView->sort(index, [](const tgui::String& a, const tgui::String& b) {
					float aFloat = a.toFloat();
					float bFloat = b.toFloat();
					return aFloat > bFloat;
				});
			});

			listView->onItemSelect([listView, &state, &constellation](int index) {
				if (index == -1) return;
				std::pair<int, int> starPlanetPair = listView->getItemData<std::pair<int, int>>(index); // IDs
				Star* star = constellation.getStarByID(starPlanetPair.first);
				if (star != nullptr) {
					Planet* planet = star->getPlanetByID(starPlanetPair.second);
					if (planet != nullptr) {
						if (state.getState() == GameState::State::LOCAL_VIEW) {
							state.changeToWorldView();
							state.changeToLocalView(star);
						}
						else {
							state.changeToLocalView(star);
						}
						state.getCamera().setPos(planet->getPos());
					}
				}
			});

			m_window->add(listView);
		}
		else {
			gui.remove(m_window);
			m_window = nullptr;
		}
	});
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