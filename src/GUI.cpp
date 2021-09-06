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

UnitGUI::UnitGUI() {
	m_mouseSelectionBox.setFillColor(sf::Color(150.0f, 150.0f, 150.0f, 100.0f));
}

void UnitGUI::open(tgui::Gui& gui) {
	m_panel = tgui::Panel::create();
	m_panel->setPosition("90%", "90%");
	m_panel->setSize("7.5%", "5%");
	m_panel->getRenderer()->setOpacity(0.75f);
	m_panel->setVisible(false);
	gui.add(m_panel);

	m_label = tgui::Label::create();
	m_label->setSize("100%", "100%");
	m_label->setText(std::to_string(m_selectedShips.size()) + "x " + "selected");
	m_panel->add(m_label);
}

void UnitGUI::update(const sf::RenderWindow& window, Star* currentStar, int playerFaction, tgui::Panel::Ptr mainPanel) {
	static bool mouseHeld = false;

	m_selectedShips.erase(std::remove_if(m_selectedShips.begin(), m_selectedShips.end(), [](Spaceship* s) {return s->isDead(); }), m_selectedShips.end());

	if (!mainPanel->isFocused()) return;

	// Mouse begins to be held down - Begin selection
	if (!mouseHeld && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		sf::Vector2i mpos = sf::Mouse::getPosition(window);
		m_mouseSelectionBox.setPosition(sf::Vector2f(mpos.x, mpos.y));
	}
	
	// Mouse held - Create selection area
	else if (mouseHeld && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		sf::Vector2i mpos = sf::Mouse::getPosition(window);
		sf::Vector2f selectionOriginPos = m_mouseSelectionBox.getPosition();

		sf::Vector2f newSize = sf::Vector2f(mpos.x - selectionOriginPos.x, mpos.y - selectionOriginPos.y);
		m_mouseSelectionBox.setSize(newSize);
	}
	
	// Mouse let go - select ships
	else if (mouseHeld && !sf::Mouse::isButtonPressed(sf::Mouse::Left) 
		&& std::abs(m_mouseSelectionBox.getSize().x) >= 5.0f 
		&& std::abs(m_mouseSelectionBox.getSize().y) >= 5.0f) {
		bool allowConstructionShips = true;

		m_selectedShips.clear();
		
		if (currentStar != nullptr) {
			for (auto& s : currentStar->getSpaceships()) {
				if (s->getCurrentStar()->isLocalViewActive() && s->getAllegiance() == playerFaction) {

					sf::Vector2i screenPos = window.mapCoordsToPixel(s->getPos());
					sf::FloatRect selection = m_mouseSelectionBox.getGlobalBounds();

					if (screenPos.x >= selection.left && screenPos.x <= selection.left + selection.width &&
						screenPos.y >= selection.top && screenPos.y <= selection.top + selection.height) {
						if (s->getConstructionSpeed() == 0.0f) {
							allowConstructionShips = false;
						}
						
						s->onSelected();
						m_selectedShips.push_back(s.get());
					}
					else if (s->isSelected()) {
						s->onDeselected();
					}
				}
			}

			// Don't mix combat and construction ships - delete from container
			if (!allowConstructionShips) {
				auto it = std::remove_if(m_selectedShips.begin(), m_selectedShips.end(), [](Spaceship* ship) {
					if (ship->getConstructionSpeed() > 0.0f) {
						ship->onDeselected();
						return true;
					}
					return false;
				});

				m_selectedShips.erase(it, m_selectedShips.end());
			}
		}

		m_mouseSelectionBox.setSize(sf::Vector2f(0.0f, 0.0f));
	}
	
	// Mouse click - deselect ships or select individual unit
	else if (mouseHeld && !sf::Mouse::isButtonPressed(sf::Mouse::Left) && m_mouseSelectionBox.getSize().x < 5.0f && m_mouseSelectionBox.getSize().y < 5.0f) {
		for (Spaceship* s : m_selectedShips) {
			if (s->isSelected()) {
				s->onDeselected();
			}
		}
		m_selectedShips.clear();

		// Select an individual unit

		if (currentStar != nullptr) {

			sf::Vector2i screenPos = sf::Mouse::getPosition(window);
			sf::Vector2f worldClick = window.mapPixelToCoords(screenPos);

			for (auto& s : currentStar->getSpaceships()) {
				if (s->getCollider().getGlobalBounds().contains(worldClick)) {
					if (s->getAllegiance() == playerFaction) {
						m_selectedShips.push_back(s.get());
						s->onSelected();
						break;
					}
				}
			}

		}
	}

	mouseHeld = sf::Mouse::isButtonPressed(sf::Mouse::Left);

	if (m_selectedShips.size() > 0) {
		m_panel->setVisible(true);
		m_label->setText(std::to_string(m_selectedShips.size()) + "x " + "selected");
	}
	else {
		m_panel->setVisible(false);
	}
}

void UnitGUI::draw(sf::RenderWindow& window) {
	sf::View oldView = window.getView();
	window.setView(window.getDefaultView());
	
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
		window.draw(m_mouseSelectionBox);
	}

	window.setView(oldView);
}

void UnitGUI::onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state, std::vector<std::unique_ptr<Star>>& stars, tgui::Panel::Ptr mainPanel) {
	if (mainPanel != nullptr) {
		if (!mainPanel->isFocused()) return;
	}

	if (state.getState() == GameState::State::LOCAL_VIEW) {
		if (ev.type == sf::Event::MouseButtonPressed) {
			if (ev.mouseButton.button == sf::Mouse::Right) {
				// Give orders to selected ships

				if (m_selectedShips.size() > 0) {
					sf::Vector2i screenPos = sf::Mouse::getPosition(window);
					sf::Vector2f worldClick = window.mapPixelToCoords(screenPos);
					JumpPoint* jumpPoint = nullptr;
					Spaceship* attackTarget = nullptr;
					Building* buildingClick = nullptr;
					Planet* planetClick = nullptr;

					// Check if click was on a jump point
					for (JumpPoint& j : state.getLocalViewStar()->getJumpPoints()) {
						if (j.isPointInRadius(worldClick)) {
							jumpPoint = &j;
							break;
						}
					}

					// Check if click was on an enemy ship
					for (auto& s : state.getLocalViewStar()->getSpaceships()) {
						if (s->getAllegiance() != m_selectedShips[0]->getAllegiance()) {
							if (s->getCollider().getGlobalBounds().contains(worldClick)) {
								attackTarget = s.get();
								break;
							}
						}
					}

					// Check if click was on a building
					for (auto& building : state.getLocalViewStar()->getBuildings()) {
						if (building->getCollider().getGlobalBounds().contains(worldClick)) {
							buildingClick = building.get();
						}
					}

					// Planets
					for (auto& planet : state.getLocalViewStar()->getPlanets()) {
						if (Math::distance(worldClick, planet.getPos()) < planet.getRadius()) {
							planetClick = &planet;
						}
					}

					// Add orders
					for (Spaceship* s : m_selectedShips) {
						if (!s->canPlayerGiveOrders()) continue;
						if (state.getLocalViewStar() == s->getCurrentStar()) {
							if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) s->clearOrders();
							if (attackTarget != nullptr) {
								s->addOrder(AttackOrder(attackTarget));
							}
							else if (buildingClick != nullptr) {
								if (buildingClick->getAllegiance() != s->getAllegiance()) {
									s->addOrder(AttackOrder(buildingClick)); // Attack enemy building
								}
								else {
									s->addOrder(InteractWithBuildingOrder(buildingClick)); // Interact with friendly building
								}
							}
							else if (jumpPoint != nullptr) {
								s->addOrder(JumpOrder(jumpPoint));
							}
							else if (planetClick != nullptr) {
								s->addOrder(InteractWithPlanetOrder(planetClick, state.getLocalViewStar()));
							}
							else {
								s->addOrder(FlyToOrder(worldClick));
							}
						}
					}
				}
			}
		}
	}
	else if (state.getState() == GameState::State::WORLD_VIEW) {
		if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Right) {
			sf::Vector2i screenPos = sf::Mouse::getPosition(window);
			sf::Vector2f worldClick = window.mapPixelToCoords(screenPos);
			Star* star = nullptr;

			// Check if click on star
			for (std::unique_ptr<Star>& s : stars) {
				if (s->isInShapeRadius(worldClick.x, worldClick.y)) {
					star = s.get();
					break;
				}
			}

			// Add travel order to star
			if (star != nullptr) {
				for (Spaceship* s : m_selectedShips) {
					if (!s->canPlayerGiveOrders()) continue;
					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) s->clearOrders();
					s->addOrder(TravelOrder(star));
				}
				if (m_selectedShips.size() > 0) {
					drawStarPath(m_selectedShips[0]->getCurrentStar(), star);
				}
			}
		}
	}
}

void UnitGUI::drawStarPath(Star* begin, Star* end) {
	std::list<Star*> path = Pathfinder::findPath(begin, end);
	
	while (path.size() > 1) {
		Star* frontStar = path.front();
		path.pop_front();

		for (JumpPoint& j : frontStar->getJumpPoints()) {
			if (j.getConnectedOtherStar() == path.front()) {
				j.getHyperlane()->enablePathEffect();
			}
		}
	}
}

OptionsMenu::OptionsMenu() {
	toml::table table;
	try {
		table = toml::parse_file("data/config/config.toml");
	}
	catch (const toml::parse_error& err) {
		DEBUG_PRINT("Failed to open config file: " << err);
	}

	m_settings.resolution = table["video"]["resolution"].value_or("1920x1080");
	m_settings.fullscreen = table["video"]["fullscreen"].value_or(true);
	m_settings.audioVolume = table["audio"]["volume"].value_or(100.0f);

	saveSettingsToFile();
}

void OptionsMenu::saveSettingsToFile() {
	toml::table config;

	toml::table video;
	video.insert("resolution", m_settings.resolution);
	video.insert("fullscreen", m_settings.fullscreen);

	toml::table audio;
	audio.insert("volume", m_settings.audioVolume);
	
	config.insert("video", video);
	config.insert("audio", audio);

	std::ofstream file("data/config/config.toml");
	file << config;
}

void OptionsMenu::open(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu) {
	auto guiWindow = tgui::ChildWindow::create("Options");
	guiWindow->setSize("50%", "50%");
	guiWindow->setPosition("(parent.size - size) / 2");
	guiWindow->setInheritedOpacity(0.75f);
	guiWindow->setTitleButtons(tgui::ChildWindow::TitleButton::None);
	gui.add(guiWindow);

	m_window = guiWindow;

	auto tabs = tgui::Tabs::create();
	tabs->setPosition("0%", "0%");
	tabs->add("Video");
	tabs->add("Audio", false);
	tabs->onTabSelect(&OptionsMenu::onTabChange, this, std::ref(gui));
	guiWindow->add(tabs, "optionsTabs");

	auto group = tgui::Group::create();
	group->setPosition("0%", "optionsTabs.height");
	group->setSize("100%", "100% - optionsTabs.height");
	guiWindow->add(group, "optionsGroup");

	auto mainMenuButton = tgui::Button::create("<- Main Menu");
	mainMenuButton->setPosition("5%", "90%");
	mainMenuButton->onPress([this, &gui, &constellation, &state, mainMenu]() {
		close(gui);
		mainMenu->open(gui, constellation, state);
	});
	guiWindow->add(mainMenuButton);

	m_group = group;

	onTabChange(gui);
}

void OptionsMenu::close(tgui::Gui& gui) {
	changeSettings(gui);
	saveSettingsToFile();
	if (m_window != nullptr) m_window->close();
}

void OptionsMenu::onTabChange(tgui::Gui& gui) {
	changeSettings(gui);
	saveSettingsToFile();
	
	auto tabs = gui.get<tgui::Tabs>("optionsTabs");
	m_group->removeAllWidgets();

	if (tabs->getSelected() == "Video") {
		auto resolutionLabel = tgui::Label::create("Resolution");
		resolutionLabel->setPosition("0%, 0%");
		m_group->add(resolutionLabel);
		
		auto dropdown = tgui::ComboBox::create();
		dropdown->setPosition("0%", "5%");
		dropdown->addItem("1920x1080");
		dropdown->addItem("1600x900");
		dropdown->addItem("1366x768");
		dropdown->addItem("1280x720");
		dropdown->addItem("960x540");
		dropdown->addItem("720x480");
		dropdown->setSelectedItem(m_settings.resolution);
		m_group->add(dropdown, "optionsResolution");

		auto fullscreenCheckbox = tgui::CheckBox::create("Fullscreen");
		fullscreenCheckbox->setPosition("0%", "15%");
		fullscreenCheckbox->setChecked(m_settings.fullscreen);
		m_group->add(fullscreenCheckbox, "fullscreenCheckbox");
	}
	else if (tabs->getSelected() == "Audio") {
		auto soundLabel = tgui::Label::create("Sound Volume");
		soundLabel->setPosition("0%, 0%");
		m_group->add(soundLabel);

		auto slider = tgui::Slider::create(0.0f, 100.0f);
		slider->setPosition("0%", "5%");
		slider->setValue(m_settings.audioVolume);
		m_group->add(slider, "optionsAudioVolume");
	}
}

void OptionsMenu::changeSettings(tgui::Gui& gui) {
	auto resolution = gui.get<tgui::ComboBox>("optionsResolution");
	auto audioVolume = gui.get<tgui::Slider>("optionsAudioVolume");
	auto fullscreen = gui.get<tgui::CheckBox>("fullscreenCheckbox");

	if (resolution != nullptr) {
		if (resolution->getSelectedItem() != m_settings.resolution) {
			m_displayChanged = true;
		}
		m_settings.resolution = resolution->getSelectedItem().toStdString();
	}
	if (audioVolume != nullptr) m_settings.audioVolume = audioVolume->getValue();
	if (fullscreen != nullptr) {
		if (fullscreen->isChecked() != m_settings.fullscreen) {
			m_displayChanged = true;
		}
		m_settings.fullscreen = fullscreen->isChecked();
	}

	m_updateGameSettings = true;
}

void OptionsMenu::updateGameSettings(sf::RenderWindow& window, Background& background, tgui::Gui& gui, EffectsEmitter& emitter, bool force) {
	if (m_updateGameSettings || force) {
		if (m_displayChanged) {
			sf::Vector2i res = getResolution();
			DEBUG_PRINT("Changing resolution to " << res.x << "x" << res.y);

			if (m_settings.fullscreen) {
				window.create(sf::VideoMode(res.x, res.y), "At Odds", sf::Style::Fullscreen);
			}
			else {
				window.create(sf::VideoMode(res.x, res.y), "At Odds", sf::Style::Titlebar | sf::Style::Close);
			}
			
			window.setFramerateLimit(60);
			gui.setTarget(window);

			float nebulaSeed = background.getNebulaSeed();
			background = Background(background.getTexturePath(), res.x, res.y);
			background.setNebulaSeed(nebulaSeed);

			emitter.init(res);

			m_displayChanged = false;
		}

		Sounds::setGlobalVolume(m_settings.audioVolume / 100.0f);

		m_updateGameSettings = false;
	}
}

sf::Vector2i OptionsMenu::getResolution() {
	std::string res = m_settings.resolution;
	int delimiterPos = res.find('x');
	int x = std::stoi(res.substr(0, delimiterPos));
	int y = std::stoi(res.substr(static_cast<size_t>(delimiterPos) + 1));

	return sf::Vector2i(x, y);
}

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
}

void BuildGUI::onBuildIconMouseExit() {
	m_buildIconPanel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
	m_buildIconPanel->getRenderer()->setOpacity(0.75f);
}

#define addBuildingSelectorIfChecked(X) if (BuildingPrototype::meetsDisplayRequirements(X, playerFaction)) addBuildingSelector(X);

void BuildGUI::onBuildIconClick(tgui::Gui& gui, Faction* playerFaction) {
	if (m_buildPanel == nullptr) {
		m_buildPanel = tgui::Panel::create();
		m_buildPanel->setInheritedOpacity(0.75);
		m_buildPanel->setPosition("2.5%", "66%");
		m_buildPanel->setSize("20%", "29%");
		gui.add(m_buildPanel);

		addBuildingSelectorIfChecked("OUTPOST")
		addBuildingSelectorIfChecked("SHIP_FACTORY")
		addBuildingSelectorIfChecked("LASER_TURRET")
		addBuildingSelectorIfChecked("MACHINE_GUN_TURRET")
		addBuildingSelectorIfChecked("GAUSS_TURRET")
		addBuildingSelectorIfChecked("ROCKET_TURRET")
		addBuildingSelectorIfChecked("FLAK_TURRET")
		
		m_selectedBuildingIdx = -1;
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
		yPosPercent = std::to_string(18 * y) + "%";
	}
	else {
		xPosPercent = "0%";
		yPosPercent = "0%";
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
	m_canReceiveEvents = false;
}

void BuildGUI::onBuildingSelectorMouseExit(int selectorIdx) {
	m_buildingSelectors[selectorIdx].panel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
	m_buildingSelectors[selectorIdx].panel->getRenderer()->setOpacity(0.75f);
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

void BuildGUI::draw(sf::RenderWindow& window, const Star* currentStar, const Player& player) {
	if (m_selectedBuildingIdx > -1 && m_buildingSelectors.size() > 0) {
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

		m_buildingSelectors[m_selectedBuildingIdx].prototype.setPos(worldPos);
		m_buildingSelectors[m_selectedBuildingIdx].prototype.draw(window, currentStar, player);
	}
}

void BuildGUI::onEvent(const sf::Event& ev, const sf::RenderWindow& window, Star* currentLocalStar, Faction* playerFaction, UnitGUI& unitGUI, tgui::Panel::Ptr mainPanel) {
	if (m_canReceiveEvents) {
		if (ev.type == sf::Event::EventType::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left) {
			if (m_selectedBuildingIdx > -1 && currentLocalStar != nullptr && m_buildingSelectors.size() > 0) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

				// Create new building
				BuildingSelector& selector = m_buildingSelectors[m_selectedBuildingIdx];

				Building* buildingPtr = nullptr;

				if (Building::checkBuildCondition(selector.prototype.getType(), currentLocalStar, playerFaction->getID(), true)) {
					std::unique_ptr<Building> building = std::make_unique<Building>(selector.prototype.getType(), currentLocalStar, worldPos, playerFaction, false);

					buildingPtr = currentLocalStar->createBuilding(building);

					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
						m_selectedBuildingIdx = -1;
					}
				}

				// Order any selected construction ships to build
				if (buildingPtr != nullptr) {
					for (Spaceship* ship : unitGUI.getSelectedShips()) {
						if (ship->isConstructor()) {
							if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
								ship->addOrder(InteractWithBuildingOrder(buildingPtr));
								mainPanel->setFocused(false);
							}
							else {
								ship->clearOrders();
								ship->addOrder(InteractWithBuildingOrder(buildingPtr));
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

void PlanetGUI::open(tgui::Gui& gui, GameState& state, Faction* playerFaction) {
	if (m_planetPanel != nullptr) {
		gui.remove(m_planetPanel);
		m_planetPanel = nullptr;
	}
	
	m_planetIconPanel = tgui::Panel::create();
	m_planetIconPanel->setPosition("0%", "85%");
	m_planetIconPanel->setSize("2.5%", "5%");
	m_planetIconPanel->getRenderer()->setOpacity(0.75f);

	m_planetIconPanel->onMouseEnter([this]() {
		m_planetIconPanel->getRenderer()->setBackgroundColor(tgui::Color::White);
	});

	m_planetIconPanel->onMouseLeave([this]() {
		m_planetIconPanel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
		m_planetIconPanel->getRenderer()->setOpacity(0.75f);
	});

	m_planetIconPanel->onClick([this, &gui, &state, playerFaction]() {
		if (m_planetPanel == nullptr) {
			if (state.getLocalViewStar() != nullptr) {
				m_planetPanel = tgui::Panel::create();
				m_planetPanel->setInheritedOpacity(0.75);
				m_planetPanel->setPosition("2.5%", "61%");
				m_planetPanel->setSize("20%", "29%");
				gui.add(m_planetPanel, "planetPanel");

				m_planetInfoPanel = tgui::Panel::create();
				m_planetInfoPanel->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
				m_planetInfoPanel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
				m_planetInfoPanel->getRenderer()->setBorders(1.0f);
				m_planetInfoPanel->setPosition("0%", "20%");
				m_planetInfoPanel->setSize("100%", "80%");
				m_planetPanel->add(m_planetInfoPanel);

				std::vector<Planet>& planets = state.getLocalViewStar()->getPlanets();

				auto planetList = tgui::ComboBox::create();
				planetList->setPosition("25%", "5%");
				planetList->setSize("50%", "10%");
				planetList->onItemSelect([this, planetList, &state, &gui, playerFaction]() {
					setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getSelectedItemIndex());
				});
				m_planetPanel->add(planetList, "planetList");

				// Add planets to dropdown list if star is discovered
				int playerAllegiance = -1;
				if (playerFaction != nullptr) {
					playerAllegiance = playerFaction->getID();
				}

				if (state.getLocalViewStar()->isDiscovered(playerAllegiance)) {
					for (int i = 0; i < planets.size(); i++) {
						planetList->addItem(std::to_string(i + 1) + ": " + planets[i].getTypeString());
					}
				}

				// Set first planet as selected
				if (planetList->getItemCount() > 0) {
					setSelectedPlanet(planetList, state, playerFaction, gui, 0);
				}
				
				auto backButton = tgui::Button::create("<-");
				backButton->setPosition("5%", "5%");
				backButton->onClick([this, planetList, &state, &gui, playerFaction]() {
					if (planetList->getItemCount() > 1) {
						if (planetList->getSelectedItemIndex() == 0) {
							setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getItemCount() - 1);
						}
						else {
							setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getSelectedItemIndex() - 1);
						}
					}
				});
				m_planetPanel->add(backButton);

				auto forwardButton = tgui::Button::create("->");
				forwardButton->setPosition("84%", "5%");
				forwardButton->onClick([this, planetList, &state, &gui, playerFaction]() {
					if (planetList->getItemCount() > 1) {
						if (planetList->getSelectedItemIndex() == planetList->getItemCount() - 1) {
							setSelectedPlanet(planetList, state, playerFaction, gui, 0);
						}
						else {
							setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getSelectedItemIndex() + 1);
						}
					}
				});
				m_planetPanel->add(forwardButton);
			}
		}
		else {
			gui.remove(m_planetPanel);
			m_planetPanel = nullptr;

			if (m_sideWindow != nullptr) {
				gui.remove(m_sideWindow);
				m_sideWindow = nullptr;
			}
		}
	});

	state.addOnChangeStateCallback([this, &gui]() {
		if (m_planetPanel != nullptr) {
			gui.remove(m_planetPanel);
			m_planetPanel = nullptr;
		}
		if (m_sideWindow != nullptr) {
			gui.remove(m_sideWindow);
			m_sideWindow = nullptr;
		}
	});

	gui.add(m_planetIconPanel);

	auto picture = tgui::Picture::create("data/art/planetsicon.png");
	picture->setSize("100%", "100%");
	m_planetIconPanel->add(picture);
}

void PlanetGUI::setSelectedPlanet(tgui::ComboBox::Ptr planetList, GameState& state, Faction* playerFaction, tgui::Gui& gui, int index) {
	planetList->setSelectedItemByIndex(index);
	m_planetInfoPanel->removeAllWidgets();
	
	gui.remove(m_sideWindow);
	m_sideWindow = nullptr;

	Planet& planet = state.getLocalViewStar()->getPlanets()[index];

	// Labels

	auto planetTypeLabel = tgui::Label::create();
	planetTypeLabel->setText("Type: " + planet.getTypeString());
	planetTypeLabel->setPosition("0%", "5%");
	m_planetInfoPanel->add(planetTypeLabel);

	auto planetTemperatureLabel = tgui::Label::create();
	planetTemperatureLabel->setText("Temperature: " + std::to_string(planet.getTemperature()));
	planetTemperatureLabel->setPosition("0%", "15%");
	m_planetInfoPanel->add(planetTemperatureLabel);

	auto planetAtmosLabel = tgui::Label::create();
	planetAtmosLabel->setText("Atmosphere: " + std::to_string(planet.getAtmosphericPressure()));
	planetAtmosLabel->setPosition("0%", "25%");
	m_planetInfoPanel->add(planetAtmosLabel);

	auto planetWaterLabel = tgui::Label::create();
	planetWaterLabel->setText("Water: " + std::to_string(planet.getWater()));
	planetWaterLabel->setPosition("0%", "35%");
	m_planetInfoPanel->add(planetWaterLabel);

	auto planetHabitabilityLabel = tgui::Label::create();
	planetHabitabilityLabel->setText("Habitability: " + std::to_string(planet.getHabitability()));
	planetHabitabilityLabel->setPosition("0%", "45%");
	m_planetInfoPanel->add(planetHabitabilityLabel);

	// Buttons

	createColonyAndResourcesButtons(gui, state, planet);

	if (state.getPlayer().getFaction() != -1) {
		createLawsButton(gui, state, planet);
	}

	createBuildingsButton(gui, planet, playerFaction);

	// Focus camera
	state.getCamera().setPos(planet.getPos());
}

void PlanetGUI::update(GameState& state) {
	if (m_planetPanel != nullptr) {
		auto planetList = m_planetPanel->get<tgui::ComboBox>("planetList");
		if (planetList->getItemCount() > 0) {
			Planet& planet = state.getLocalViewStar()->getPlanets()[planetList->getSelectedItemIndex()];

			// Lock camera on planet
			state.getCamera().setPos(planet.getPos());
		}

		if (m_sideWindow != nullptr) {
			if (m_sideWindow->getTitle() == "Buildings") {
				auto buildingsBox = m_sideWindow->get<tgui::ListBox>("buildingsBox");
				
				if (buildingsBox->getSelectedItemIndex() != -1) {
					Planet& planet = state.getLocalViewStar()->getPlanets()[planetList->getSelectedItemIndex()];

					for (ColonyBuilding& building : planet.getColony().getBuildings()) {
						if (building.getName() == buildingsBox->getSelectedItem()) {
							// Update progress bar
							m_sideWindow->get<tgui::ProgressBar>("buildProgressBar")->setValue(building.getPercentBuilt());
						}
					}

				}
			}
		}
	}
}

void PlanetGUI::switchSideWindow(const std::string& name, tgui::Gui& gui) {
	if (m_sideWindow == nullptr) {
		m_sideWindow = tgui::ChildWindow::create(name);
		m_sideWindow->setOrigin(0.0f, 1.0f);
		m_sideWindow->setPosition("planetPanel.right", "planetPanel.bottom");
		m_sideWindow->setSize("20%", "29%");
		m_sideWindow->getRenderer()->setOpacity(0.75f);
		m_sideWindow->setPositionLocked(true);
		m_sideWindow->onClose([this]() {
			m_sideWindow = nullptr;
		});
		gui.add(m_sideWindow);
	}
	else {
		if (m_sideWindow->getTitle() != name) {
			m_sideWindow->setTitle(name);
			m_sideWindow->setSize("20%", "29%");
			m_sideWindow->removeAllWidgets();
		}
		else {
			m_sideWindow->close();
		}
	}
}

void PlanetGUI::onEvent(const sf::Event& ev, tgui::Gui& gui, GameState& state, Faction* playerFaction, const sf::RenderWindow& window, Star* currentStar, tgui::Panel::Ptr mainPanel) {
	if (currentStar != nullptr && m_planetIconPanel != nullptr && mainPanel->isFocused()) {
		static sf::Vector2f lastMousePressPos;

		if (ev.type == sf::Event::EventType::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left) {
			sf::Vector2i screenPos = sf::Mouse::getPosition(window);
			sf::Vector2f worldPos = window.mapPixelToCoords(screenPos);

			int i = 0;
			for (Planet& planet : currentStar->getPlanets()) {
				if (Math::distance(worldPos, planet.getPos()) < planet.getRadius() &&
					Math::distance(lastMousePressPos, planet.getPos()) < planet.getRadius()) {
					// Open planet panel
					if (m_planetPanel == nullptr) {
						m_planetIconPanel->onClick.emit(m_planetIconPanel.get(), worldPos);
					}
					setSelectedPlanet(m_planetPanel->get<tgui::ComboBox>("planetList"), state, playerFaction, gui, i);
					break;
				}
				i++;
			}
		}
		else if (ev.type == sf::Event::EventType::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
			lastMousePressPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		}
	}
}

void PlanetGUI::createColonyAndResourcesButtons(tgui::Gui& gui, GameState& state, Planet& planet) {
	// Colony button
	auto colonyInfoButton = tgui::Button::create();
	colonyInfoButton->setPosition("75%", "80%");
	colonyInfoButton->setText("Colony");
	colonyInfoButton->setSize("25%", "10%");
	auto openColonyInfo = [this, &gui, &state, &planet]() {
		switchSideWindow("Colony", gui);

		if (m_sideWindow == nullptr) return;

		Colony& colony = planet.getColony();

		auto populationLabel = tgui::Label::create("Population: " + std::to_string(colony.getPopulation()));
		m_sideWindow->add(populationLabel);

		if (colony.getPopulation() > 0) {
			auto allegianceLabel = tgui::Label::create("Allegiance: ");
			allegianceLabel->setPosition("0%", "10%");
			m_sideWindow->add(allegianceLabel, "allegianceLabel");

			auto allegianceText = tgui::Label::create();
			if (state.getPlayer().getFaction() == colony.getAllegiance()) {
				allegianceText->setText("Friendly");
				allegianceText->getRenderer()->setTextColor(tgui::Color::Green);
			}
			else {
				allegianceText->setText("Hostile");
				allegianceText->getRenderer()->setTextColor(tgui::Color::Red);
			}
			allegianceText->setPosition("allegianceLabel.right", "allegianceLabel.top");
			m_sideWindow->add(allegianceText);

			auto growthRateLabel = tgui::Label::create("Growth Rate: " + std::to_string(colony.getGrowthRate(planet.getHabitability()) * 100.0f) + "%");
			growthRateLabel->setPosition("0%", "20%");
			m_sideWindow->add(growthRateLabel);
		}
	};
	colonyInfoButton->onClick(openColonyInfo);
	m_planetInfoPanel->add(colonyInfoButton, "colonyInfoButton");

	// Resources button
	auto resourceInfoButton = tgui::Button::create("Resources");
	resourceInfoButton->setPosition("colonyInfoButton.left", "colonyInfoButton.top - 10.0%");
	resourceInfoButton->setTextSize(10);
	resourceInfoButton->setSize("25%", "10%");
	auto openResourceInfo = [this, &gui, &planet]() {
		switchSideWindow("Resources", gui);

		if (m_sideWindow == nullptr) return;

		auto abundanceLabel = tgui::Label::create();
		abundanceLabel->setPosition("50%", "0%");
		abundanceLabel->setText("");
		m_sideWindow->add(abundanceLabel, "abundanceLabel");

		auto resourceListBox = tgui::ListBox::create();
		resourceListBox->setPosition("2.5%", "5%");
		resourceListBox->setSize("47.5%", "90%");

		for (auto& resource : planet.getResources()) {
			resourceListBox->addItem(resource.getName());
		}

		resourceListBox->onItemSelect([this, &planet]() {
			auto listBox = m_sideWindow->get<tgui::ListBox>("resourceListBox");
			auto abundanceLabel = m_sideWindow->get<tgui::Label>("abundanceLabel");
			if (listBox->getSelectedItemIndex() >= 0 && listBox->getSelectedItemIndex() < listBox->getItemCount()) {
				abundanceLabel->setText("Abundance: " + std::to_string(planet.getResources()[listBox->getSelectedItemIndex()].abundance));
			}
			});

		m_sideWindow->add(resourceListBox, "resourceListBox");
	};

	if (planet.getColony().getPopulation() > 0) {
		openColonyInfo();
	}
	else {
		openResourceInfo();
	}

	resourceInfoButton->onClick(openResourceInfo);
	m_planetInfoPanel->add(resourceInfoButton);

}

void PlanetGUI::createLawsButton(tgui::Gui& gui, GameState& state, Planet& planet) {
	auto lawsButton = tgui::Button::create("Laws");
	lawsButton->setPosition("colonyInfoButton.left", "colonyInfoButton.top - 20.0%");
	lawsButton->setSize("25%", "10%");
	lawsButton->onPress([this, &gui, &state, &planet]() {
		switchSideWindow("Laws", gui);

		if (m_sideWindow == nullptr) return;

		auto colonyLawLabel = tgui::Label::create("Colonization Legality: ");
		m_sideWindow->add(colonyLawLabel, "colonyLawLabel");

		auto colonyLawComboBox = tgui::ComboBox::create();
		colonyLawComboBox->setPosition("colonyLawLabel.right", "colonyLawLabel.top");
		colonyLawComboBox->setSize("40%", "10%");
		colonyLawComboBox->addItem("Illegal");
		colonyLawComboBox->addItem("Legal");
		colonyLawComboBox->onItemSelect([this, colonyLawComboBox, &planet, &state]() {
			if (colonyLawComboBox->getSelectedItem() == "Illegal") {
				planet.getColony().setFactionColonyLegality(state.getPlayer().getFaction(), false);
			}
			else if (colonyLawComboBox->getSelectedItem() == "Legal") {
				planet.getColony().setFactionColonyLegality(state.getPlayer().getFaction(), true);
			}
			});

		if (planet.getColony().isColonizationLegal(state.getPlayer().getFaction())) {
			colonyLawComboBox->setSelectedItemByIndex(1);
		}
		else {
			colonyLawComboBox->setSelectedItemByIndex(0);
		}

		m_sideWindow->add(colonyLawComboBox);
		});
	m_planetInfoPanel->add(lawsButton, "lawsButton");
}

void PlanetGUI::createBuildingsButton(tgui::Gui& gui, Planet& planet, Faction* playerFaction) {
	auto buildingsButton = tgui::Button::create("Buildings");
	buildingsButton->setPosition("colonyInfoButton.left", "colonyInfoButton.top - 30.0%");
	buildingsButton->setSize("25%", "10%");
	buildingsButton->onPress([this, &gui, &planet, playerFaction]() {
		switchSideWindow("Buildings", gui);

		if (m_sideWindow == nullptr) return;

		m_sideWindow->setSize(m_sideWindow->getSize() * 2.0f);

		auto buildingsBox = tgui::ListBox::create();
		buildingsBox->setPosition("2.5%", "5%");
		buildingsBox->setSize("47.5%", "90%");
		m_sideWindow->add(buildingsBox, "buildingsBox");

		auto buildings = planet.getColony().getBuildings();

		for (ColonyBuilding& building : buildings) {
			buildingsBox->addItem(building.getName());
		}

		buildingsBox->onItemSelect([this, buildingsBox, &planet]() {
			auto infoGroup = m_sideWindow->get<tgui::Group>("infoGroup");
			if (infoGroup != nullptr) {
				m_sideWindow->remove(infoGroup);
			}

			if (buildingsBox->getSelectedItemIndex() != -1) {
				auto& allBuildings = planet.getColony().getBuildings();
				for (ColonyBuilding& building : allBuildings) {
					if (building.getName() == buildingsBox->getSelectedItem()) {
						displayBuildingInfo(building, planet, false);
						break;
					}
				}
			}
		});

		int playerAllegiance = -1;
		if (playerFaction != nullptr) {
			playerAllegiance = playerFaction->getID();
		}

		if (planet.getColony().getAllegiance() == playerAllegiance && playerAllegiance != -1) {
			auto buildButton = tgui::Button::create("Build");
			buildButton->setPosition("buildingsBox.right + 2.5%", "85%");
			buildButton->onPress([this, &gui, &planet, playerFaction]() {
				switchSideWindow("Build Colony Buildings", gui);

				if (m_sideWindow == nullptr) return;

				m_sideWindow->setSize(m_sideWindow->getSize() * 2.0f);

				auto listBox = tgui::ListBox::create();
				listBox->setPosition("2.5%", "5%");
				listBox->setSize("47.5%", "90%");
				m_sideWindow->add(listBox);

				auto buildings = playerFaction->getColonyBuildings();

				// Add buildable buildings to listbox
				for (const ColonyBuilding& building : buildings) {
					listBox->addItem(building.getName());
				}

				listBox->onItemSelect([this, listBox, &planet, playerFaction]() {
					auto infoGroup = m_sideWindow->get<tgui::Group>("infoGroup");
					if (infoGroup != nullptr) {
						m_sideWindow->remove(infoGroup);
					}

					if (listBox->getSelectedItemIndex() != -1) {
						auto allBuildings = playerFaction->getColonyBuildings();
						for (ColonyBuilding& building : allBuildings) {
							if (building.getName() == listBox->getSelectedItem()) {
								displayBuildingInfo(building, planet, true);

								if (!planet.getColony().hasBuildingOfType(building.getType())) {
									auto buildButton = tgui::Button::create("Build");
									buildButton->setPosition("2.5%", "85%");
									m_sideWindow->get<tgui::Group>("infoGroup")->add(buildButton);

									buildButton->onPress([this, buildButton, building, &planet, playerFaction]() {
										if (!planet.getColony().hasBuildingOfType(building.getType())) {
											auto cost = building.getResourceCost(planet);

											if (playerFaction->canSubtractResources(cost)) {
												planet.getColony().addBuilding(building);
												playerFaction->subtractResources(cost);

												m_sideWindow->get<tgui::Group>("infoGroup")->remove(buildButton);
												createBuildStatusLabel(planet, building);
											}
										}
									});
								}
								else {
									createBuildStatusLabel(planet, building);
								}

								break;
							}
						}
					}
					});
				});
			m_sideWindow->add(buildButton);
		}
	});
	m_planetInfoPanel->add(buildingsButton);
}

void PlanetGUI::displayBuildingInfo(ColonyBuilding& building, Planet& planet, bool buildInfo) {
	auto infoGroup = m_sideWindow->get<tgui::Group>("infoGroup");
	if (infoGroup != nullptr) {
		m_sideWindow->remove(infoGroup);
	}

	infoGroup = tgui::Group::create();
	infoGroup->setPosition("50%", "0%");
	infoGroup->setSize("50%", "100%");
	m_sideWindow->add(infoGroup, "infoGroup");

	auto nameLabel = tgui::Label::create(building.getName());
	nameLabel->setAutoSize(true);
	nameLabel->getRenderer()->setBorders(1);
	nameLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
	infoGroup->add(nameLabel, "nameLabel");

	auto descriptionLabel = tgui::Label::create(building.getDescription());
	descriptionLabel->setPosition("0%", "nameLabel.bottom");
	descriptionLabel->setSize("100%", "20%");
	descriptionLabel->getRenderer()->setBorders(1);
	descriptionLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
	infoGroup->add(descriptionLabel, "descriptionLabel");

	auto effectsLabel = tgui::Label::create(building.getEffectsString());
	effectsLabel->setPosition("0%", "descriptionLabel.bottom");
	effectsLabel->setSize("100%", "20%");
	effectsLabel->getRenderer()->setBorders(1);
	effectsLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
	infoGroup->add(effectsLabel, "effectsLabel");

	if (!buildInfo) {
		auto statusLabel = tgui::Label::create();
		if (!building.isBuilt()) {
			statusLabel->setText("Status: Under Construction");
		}
		else {
			statusLabel->setText("Status: Operational");
		}
		statusLabel->setPosition("effectsLabel.left", "effectsLabel.bottom");
		statusLabel->setSize("100%", "10%");
		statusLabel->getRenderer()->setBorders(1);
		statusLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
		infoGroup->add(statusLabel, "statusLabel");

		auto buildProgressBar = tgui::ProgressBar::create();
		buildProgressBar->setPosition("statusLabel.left", "statusLabel.bottom");
		infoGroup->add(buildProgressBar, "buildProgressBar");
	}
	else {
		auto cost = building.getResourceCost(planet);
		std::stringstream resourceStr;
		resourceStr << "Build Cost: \n";
		resourceStr << std::fixed << std::setprecision(1);
		for (auto& pair : cost) {
			Resource resource(pair.first);
			resourceStr << resource.getName() << ": " << pair.second << "\n";
		}

		auto costLabel = tgui::Label::create(resourceStr.str());
		costLabel->setPosition("effectsLabel.left", "effectsLabel.bottom");
		costLabel->setSize("100%", "10%");
		costLabel->getRenderer()->setBorders(1);
		costLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
		infoGroup->add(costLabel);
	}
}

void PlanetGUI::createBuildStatusLabel(Planet& planet, const ColonyBuilding& building) {
	auto statusLabel = tgui::Label::create();
	statusLabel->setPosition("2.5%", "85%");
	if (planet.getColony().getBuildingOfType(building.getType())->isBuilt()) {
		statusLabel->setText("Built");
	}
	else {
		statusLabel->setText("Under Construction");
	}
	m_sideWindow->get<tgui::Group>("infoGroup")->add(statusLabel);
}

void BuildingGUI::onEvent(const sf::Event& ev, const sf::RenderWindow& window, tgui::Gui& gui, GameState& state, Constellation& constellation, tgui::Panel::Ptr mainPanel) {
	bool valid = ev.mouseButton.button == sf::Mouse::Left && state.getLocalViewStar() != nullptr && mainPanel->isFocused();
	
	if (ev.type == sf::Event::EventType::MouseButtonReleased && valid) {
		sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
		sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mouseScreenPos);
		
		if (m_window != nullptr) {
			if (m_window->isMouseOnWidget(tgui::Vector2f(mouseScreenPos.x, mouseScreenPos.y))) {
				return;
			}
		}

		for (auto& building : state.getLocalViewStar()->getBuildings()) {
			float dist = Math::distance(building->getPos(), mouseWorldPos);
			float downDist = Math::distance(building->getPos(), m_lastMouseDownPos);
			if (dist < building->getCollider().getRadius() && state.getPlayer().getFaction() == building->getAllegiance() &&
				downDist < building->getCollider().getRadius()) {
				if (m_window != nullptr) {
					gui.remove(m_window);
				}

				m_window = tgui::ChildWindow::create();
				m_window->getRenderer()->setOpacity(0.75f);
				m_window->setSize("10%", "15%");
				m_window->setOrigin(0.5f, 0.5f);
				m_window->setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
				m_window->setTitle(building->getName());
				m_window->onClose([this]() {
					m_window = nullptr;
				});
				gui.add(m_window);

				building->openModGUI(m_window, constellation.getFaction(state.getPlayer().getFaction()));

				break;
			}
		}
	}
	else if (ev.type == sf::Event::EventType::MouseButtonPressed && valid) {
		m_lastMouseDownPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
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

void ResourceGUI::open(tgui::Gui& gui) {
	m_resourceGroup = tgui::Group::create();
	m_resourceGroup->setPosition("0%", "0%");
	m_resourceGroup->setSize("33%", "33%");
	m_resourceGroup->setFocusable(false);
	gui.add(m_resourceGroup);
}

void ResourceGUI::update(Constellation& constellation, Player& player) {
	if (m_resourceGroup != nullptr) {
		if (player.getFaction() != -1) {
			Faction* faction = constellation.getFaction(player.getFaction());
			auto& resources = faction->getResources();

			if (m_resourceGroup->get<tgui::Label>("resourceLabel") == nullptr) {
				auto resourceLabel = tgui::Label::create("Resources");
				resourceLabel->setTextSize(20);
				m_resourceGroup->add(resourceLabel, "resourceLabel");
			}

			int pos = 15;

			for (auto& resource : resources) {
				Resource r;
				r.type = resource.first;
				
				auto label = m_resourceGroup->get<tgui::Label>(r.getName());

				if (label == nullptr) {
					label = tgui::Label::create();
					label->setPosition("0%", (std::to_string(pos) + "%").c_str());
					label->setTextSize(20);
					m_resourceGroup->add(label, r.getName());
				}

				std::stringstream ss;
				ss << r.getName() << ": " << std::fixed << std::setprecision(1) << resource.second;

				label->setText(ss.str());
				pos += 15;
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
			listView->addColumn("Population");
			listView->addColumn("Growth Rate");
			listView->addColumn("Temperature");
			listView->addColumn("Water");
			listView->addColumn("Atmosphere");
			listView->addColumn("Habitability");

			int playerFaction = state.getPlayer().getFaction();
			for (auto& star : constellation.getStars()) {
				for (Planet& planet : star->getPlanets()) {
					if (planet.getColony().getAllegiance() == playerFaction) {
						std::vector<tgui::String> info;
						info.push_back(planet.getTypeString());
						info.push_back(std::to_string(planet.getColony().getPopulation()));
						info.push_back(std::to_string(planet.getColony().getGrowthRate(planet.getHabitability()) * 100.0f) + "%");
						info.push_back(std::to_string(planet.getTemperature()));
						info.push_back(std::to_string(planet.getWater()));
						info.push_back(std::to_string(planet.getAtmosphericPressure()));
						info.push_back(std::to_string(planet.getHabitability()));
						listView->addItem(info);
						listView->setItemData(listView->getItemCount() - 1, std::pair<int, int>(star->getID(), planet.getID()));
					}
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