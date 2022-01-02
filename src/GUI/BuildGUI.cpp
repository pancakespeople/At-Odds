#include "gamepch.h"
#include "BuildGUI.h"
#include "../Faction.h"
#include "../Building.h"
#include "../Star.h"
#include "UnitGUI.h"
#include "../Renderer.h"

void BuildGUI::open(tgui::Gui& gui, Faction* playerFaction) {
	auto panel = tgui::Panel::create();
	panel->setPosition("0%", "90%");
	panel->setSize("2.5%", "5%");
	panel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
	panel->getRenderer()->setOpacity(0.75f);
	panel->onClick(&BuildGUI::onBuildIconClick, this, std::ref(gui), playerFaction);
	panel->onMouseEnter(&BuildGUI::onBuildIconMouseEnter, this);
	panel->onMouseLeave(&BuildGUI::onBuildIconMouseExit, this);
	m_buildIconPanel = panel;
	gui.add(panel);

	auto picture = tgui::Picture::create("data/art/buildicon.png");
	picture->setSize("100%", "100%");
	m_buildIcon = picture;
	panel->add(picture);
}

void BuildGUI::onBuildIconMouseEnter() {
	m_buildIconPanel->getRenderer()->setBackgroundColor(tgui::Color::White);
	m_buildIconPanel->setRenderer(tgui::Theme().getRenderer("Panel"));
}

void BuildGUI::onBuildIconMouseExit() {
	m_buildIconPanel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
	m_buildIconPanel->getRenderer()->setOpacity(0.75f);
	m_buildIconPanel->setRenderer(tgui::Theme::getDefault()->getRenderer("Panel"));
}

void BuildGUI::onBuildIconClick(tgui::Gui& gui, Faction* playerFaction) {
	if (m_buildPanel == nullptr) {
		m_buildPanel = tgui::Panel::create();
		m_buildPanel->setInheritedOpacity(0.75);
		m_buildPanel->setPosition("2.5%", "66%");
		m_buildPanel->setSize("20%", "29%");
		gui.add(m_buildPanel);

		m_tabs = tgui::Tabs::create();
		m_tabs->setSize("100%", "10%");
		m_tabs->add("Defense");
		m_tabs->add("Economy");
		m_tabs->onTabSelect([this, playerFaction]() {
			updateBuildingSelectors(playerFaction);
		});
		m_buildPanel->add(m_tabs);

		updateBuildingSelectors(playerFaction);
	}
	else {
		gui.remove(m_buildPanel);
		m_buildingSelectors.clear();
		m_buildPanel = nullptr;
	}
}

void BuildGUI::addBuildingSelector(const std::string& type) {
	std::string xPosPercent;
	std::string yPosPercent;
	if (m_buildingSelectors.size() > 0) {
		int x = m_buildingSelectors.size() % 5;
		int y = m_buildingSelectors.size() / 5;
		xPosPercent = std::to_string(18 * x) + "%";
		yPosPercent = std::to_string(18 * y + 10) + "%";
	}
	else {
		xPosPercent = "0%";
		yPosPercent = "10%";
	}

	BuildingSelector selector;
	selector.panel = tgui::Panel::create();
	selector.panel->setPosition(xPosPercent.c_str(), yPosPercent.c_str());
	selector.panel->setSize("18%", "18%");
	selector.panel->onMouseEnter(&BuildGUI::onBuildingSelectorMouseEnter, this, m_buildingSelectors.size());
	selector.panel->onMouseLeave(&BuildGUI::onBuildingSelectorMouseExit, this, m_buildingSelectors.size());
	selector.panel->onClick(&BuildGUI::onBuildingSelectorClick, this, m_buildingSelectors.size());
	selector.panel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
	selector.panel->getRenderer()->setOpacity(0.75f);
	m_buildPanel->add(selector.panel);

	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");

	assert(table.contains(type));

	selector.prototype = BuildingPrototype(type);

	selector.icon = tgui::Picture::create(table[type]["texturePath"].value_or(""));
	selector.icon->setSize("100%", "100%");
	selector.panel->add(selector.icon);

	auto tooltip = tgui::Label::create(table[type]["name"].value_or(""));
	tgui::ToolTip::setInitialDelay(0);
	selector.panel->setToolTip(tooltip);

	m_buildingSelectors.push_back(selector);
}

void BuildGUI::onBuildingSelectorMouseEnter(int selectorIdx) {
	m_buildingSelectors[selectorIdx].panel->getRenderer()->setBackgroundColor(tgui::Color::White);
	m_buildingSelectors[selectorIdx].panel->setRenderer(tgui::Theme().getRenderer("Panel"));
	m_canReceiveEvents = false;
}

void BuildGUI::onBuildingSelectorMouseExit(int selectorIdx) {
	m_buildingSelectors[selectorIdx].panel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
	m_buildingSelectors[selectorIdx].panel->getRenderer()->setOpacity(0.75f);
	m_buildingSelectors[selectorIdx].panel->setRenderer(tgui::Theme::getDefault()->getRenderer("Panel"));
	m_canReceiveEvents = true;
}

void BuildGUI::onBuildingSelectorClick(int selectorIdx) {
	if (m_selectedBuildingIdx == -1) {
		m_selectedBuildingIdx = selectorIdx;
	}
	else {
		m_selectedBuildingIdx = -1;
	}
}

void BuildGUI::draw(sf::RenderWindow& window, Renderer& renderer, Star* currentStar, Faction* playerFaction) {
	if (m_selectedBuildingIdx > -1 && m_buildingSelectors.size() > 0) {
		window.setView(renderer.getView());
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

		m_buildingSelectors[m_selectedBuildingIdx].prototype.setPos(worldPos);
		m_buildingSelectors[m_selectedBuildingIdx].prototype.draw(window, currentStar, playerFaction);
		window.setView(window.getDefaultView());
	}
}

void BuildGUI::onEvent(const sf::Event& ev, const sf::RenderWindow& window, Renderer& renderer, Star* currentLocalStar, Faction* playerFaction, UnitGUI& unitGUI, tgui::Panel::Ptr mainPanel) {
	if (m_canReceiveEvents) {
		if (ev.type == sf::Event::EventType::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left) {
			if (m_selectedBuildingIdx > -1 && currentLocalStar != nullptr && m_buildingSelectors.size() > 0) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				sf::Vector2f worldPos = renderer.mapPixelToCoords(mousePos);

				// Create new building
				BuildingSelector& selector = m_buildingSelectors[m_selectedBuildingIdx];

				Building* building = nullptr;

				if (Building::checkBuildCondition(selector.prototype.getType(), selector.prototype.getPos(), selector.prototype.getRadius(), currentLocalStar, playerFaction, true)) {
					building = currentLocalStar->createBuilding(selector.prototype.getType(), worldPos, playerFaction, false);

					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
						m_selectedBuildingIdx = -1;
					}
				}

				// Order any selected construction ships to build
				if (building != nullptr) {
					for (Spaceship* ship : unitGUI.getSelectedShips()) {
						if (ship->isConstructor()) {
							if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
								ship->addOrder(InteractWithBuildingOrder(building));
								mainPanel->setFocused(false);
							}
							else {
								ship->clearOrders();
								ship->addOrder(InteractWithBuildingOrder(building));
							}
						}
					}
				}
			}
		}
		else if (ev.type == sf::Event::EventType::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
			if (mainPanel != nullptr && m_selectedBuildingIdx > -1 && currentLocalStar != nullptr && m_buildingSelectors.size() > 0) {
				mainPanel->setFocused(false);
			}
		}
		else if (ev.type == sf::Event::EventType::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Right) {
			if (m_selectedBuildingIdx > -1 && currentLocalStar != nullptr && m_buildingSelectors.size() > 0) {
				m_selectedBuildingIdx = -1;

				if (mainPanel != nullptr) {
					mainPanel->setFocused(false);
				}
			}
		}
	}
}

void BuildGUI::updateBuildingSelectors(Faction* playerFaction) {
	// Remove current panels
	for (int i = 0; i < m_buildingSelectors.size(); i++) {
		m_buildPanel->remove(m_buildingSelectors[i].panel);
	}
	m_buildingSelectors.clear();
	
	const toml::table& table = TOMLCache::getTable("data/objects/buildings.toml");
	for (auto& type : table) {
		if (!table[type.first]["cannotBuild"].value_or(false) && table[type.first]["category"].value_or("") == m_tabs->getSelected()) {
			if (BuildingPrototype::meetsDisplayRequirements(type.first, playerFaction)) {
				addBuildingSelector(type.first);
			}
		}
	}

	m_selectedBuildingIdx = -1;
}