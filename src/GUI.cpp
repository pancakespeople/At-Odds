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

void MainMenu::open(tgui::Gui& gui, Constellation& constellation, GameState& state) {
	auto panel = tgui::Panel::create();
	panel->setPosition("0%", "66%");
	panel->setSize("100%", "10%");
	panel->setInheritedOpacity(0.75f);
	gui.add(panel);
	m_panel = panel;

	m_title = tgui::Label::create();
	m_title->setOrigin(0.5f, 0.5f);
	m_title->setPosition("50%", "33%");
	m_title->setText("At Odds");
	m_title->setTextSize(250);
	m_title->getRenderer()->setTextColor(tgui::Color::Red);
	m_title->getRenderer()->setFont("data/fonts/segoesc.ttf");
	gui.add(m_title);

	auto newGameButton = tgui::Button::create("New Game");
	newGameButton->setPosition("20%", "50%");
	newGameButton->setOrigin(0.5f, 0.5f);
	newGameButton->onPress([this, &gui, &constellation, &state]() {
		close(gui);
		m_newGameMenu.open(gui, constellation, state, this);
	});
	m_panel->add(newGameButton);

	auto loadGameButton = tgui::Button::create("Load Game");
	loadGameButton->setPosition("40%", "50%");
	loadGameButton->setOrigin(0.5f, 0.5f);
	loadGameButton->onPress([&state] {
		state.loadGame();
	});
	m_panel->add(loadGameButton);

	auto optionsButton = tgui::Button::create("Options");
	optionsButton->setPosition("60%", "50%");
	optionsButton->setOrigin(0.5f, 0.5f);
	optionsButton->onPress([this, &gui, &constellation, &state]() {
		close(gui);
		m_optionsMenu.open(gui, constellation, state, this);
	});
	m_panel->add(optionsButton);

	auto exitButton = tgui::Button::create("Exit");
	exitButton->setPosition("80%", "50%");
	exitButton->setOrigin(0.5f, 0.5f);
	exitButton->onPress(&MainMenu::exitGame, this, std::ref(state));
	m_panel->add(exitButton);

	m_opened = true;
}

void MainMenu::close(tgui::Gui& gui) {
	gui.remove(m_panel);
	gui.remove(m_title);
	m_panel = nullptr;
	m_title = nullptr;
	m_opened = false;
}

void MainMenu::exitGame(GameState& state) {
	state.exitGame();
}

void MainMenu::onEvent(sf::Event& ev, tgui::Gui& gui, Constellation& constellation, GameState& state) {
	if (ev.type == sf::Event::KeyPressed) {
		if (ev.key.code == sf::Keyboard::Escape && state.getState() != GameState::State::MAIN_MENU && !m_opened) {
			m_newGameMenu.close();
			m_optionsMenu.close(gui);
			open(gui, constellation, state);
		}
		else if (ev.key.code == sf::Keyboard::Escape && state.getState() != GameState::State::MAIN_MENU && m_opened) {
			close(gui);
		}
	}
}

void NewGameMenu::open(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu) {
	auto guiWindow = tgui::ChildWindow::create("New Game");
	guiWindow->setSize("50%", "50%");
	guiWindow->setPosition("(parent.size - size) / 2");
	guiWindow->setInheritedOpacity(0.75f);
	guiWindow->setTitleButtons(tgui::ChildWindow::TitleButton::None);
	gui.add(guiWindow);
	m_window = guiWindow;

	auto mainMenuButton = tgui::Button::create("<- Main Menu");
	mainMenuButton->setPosition("5%", "90%");
	mainMenuButton->onPress([this, &gui, &constellation, &state, mainMenu]() {
		close();
		mainMenu->open(gui, constellation, state);
	});
	guiWindow->add(mainMenuButton);

	auto startGameButton = tgui::Button::create("Start Game ->");
	startGameButton->setPosition("95% - width", "90%");
	startGameButton->onPress(&NewGameMenu::startNewGame, this, std::ref(gui), std::ref(constellation), std::ref(state));
	guiWindow->add(startGameButton);

	// Stars slider

	auto numStarsLabel = tgui::Label::create("Number of stars");
	numStarsLabel->setPosition("5%", "0%");
	guiWindow->add(numStarsLabel);

	auto numStarsSlider = tgui::Slider::create(20.0f, 500.0f);
	numStarsSlider->setValue(20.0f);
	numStarsSlider->setStep(1.0f);
	numStarsSlider->setPosition("5%", "5%");
	numStarsSlider->onValueChange(&NewGameMenu::onStarsSliderChange, this, std::ref(gui));
	guiWindow->add(numStarsSlider, "starSlider");

	auto numStarsNumLabel = tgui::Label::create(std::to_string(static_cast<int>(numStarsSlider->getValue())));
	numStarsNumLabel->setPosition("starSlider.left + starSlider.width", "starSlider.top");
	guiWindow->add(numStarsNumLabel, "numStars");

	//
	// Factions slider

	auto numFactionsLabel = tgui::Label::create("Number of factions");
	numFactionsLabel->setPosition("5%", "15%");
	guiWindow->add(numFactionsLabel);

	auto numFactionsSlider = tgui::Slider::create(2.0f, 15.0f);
	numFactionsSlider->setValue(4.0f);
	numFactionsSlider->setStep(1.0f);
	numFactionsSlider->setPosition("5%", "20%");
	numFactionsSlider->onValueChange(&NewGameMenu::onFactionsSliderChange, this, std::ref(gui));
	guiWindow->add(numFactionsSlider, "factionSlider");

	auto numFactionsNumLabel = tgui::Label::create(std::to_string(static_cast<int>(numFactionsSlider->getValue())));
	numFactionsNumLabel->setPosition("factionSlider.left + factionSlider.width", "factionSlider.top");
	guiWindow->add(numFactionsNumLabel, "numFactions");

	//

	auto spectateCheckbox = tgui::CheckBox::create("Spectate");
	spectateCheckbox->setPosition("5%", "30%");
	guiWindow->add(spectateCheckbox, "spectateCheckBox");
}

void NewGameMenu::close() {
	if (m_window != nullptr) 
		m_window->close();
}

void NewGameMenu::onStarsSliderChange(tgui::Gui& gui) {
	auto slider = gui.get<tgui::Slider>("starSlider");
	auto label = gui.get<tgui::Label>("numStars");

	label->setText(std::to_string(static_cast<int>(slider->getValue())));
}

void NewGameMenu::onFactionsSliderChange(tgui::Gui& gui) {
	auto slider = gui.get<tgui::Slider>("factionSlider");
	auto label = gui.get<tgui::Label>("numFactions");

	label->setText(std::to_string(static_cast<int>(slider->getValue())));
}

void NewGameMenu::startNewGame(tgui::Gui& gui, Constellation& constellation, GameState& state) {
	int starsNum = m_window->get<tgui::Slider>("starSlider")->getValue();
	int factionsNum = m_window->get<tgui::Slider>("factionSlider")->getValue();
	
	Camera camera = state.getCamera();

	state = GameState(camera);
	constellation = Constellation();

	constellation.generateRecursiveConstellation(800, 800, starsNum);
	constellation.setupStars();

	auto& stars = constellation.getStars();

	constellation.generateFactions(factionsNum);
	constellation.generateNeutralSquatters();

	state.getCamera().setPos(stars[0]->getPos());
	state.getCamera().resetZoom();

	bool spectate = m_window->get<tgui::CheckBox>("spectateCheckBox")->isChecked();

	gui.removeAllWidgets();
	state.clearCallbacks();
	Sounds::clearSounds();

	if (!spectate) {
		constellation.getFactions()[0].controlByPlayer(state.getPlayer());
		state.changeToLocalView(constellation.getFactions()[0].getCapitol());
		state.getCamera().setPos(constellation.getFactions()[0].getCapitol()->getLocalViewCenter());

		m_playerGui.open(gui, state, constellation, false);
	}
	else {
		state.changeToWorldView();
		constellation.discoverAllStars();

		m_playerGui.open(gui, state, constellation, true);
	}

	for (auto& func : m_gameStartCallbacks) {
		func();
	}

	close();
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

void PlayerGUI::open(tgui::Gui& gui, GameState& state, Constellation& constellation, bool spectator) {
	// An invisible bottom level panel to help with gui focusing
	mainPanel = tgui::Panel::create();
	mainPanel->getRenderer()->setOpacity(0.0f);
	gui.add(mainPanel);
	
	Faction* playerFaction = constellation.getFaction(state.getPlayer().getFaction());

	if (!spectator) {
#ifdef NDEBUG
		helpWindow.open(gui);
#endif
		buildGUI.open(gui, playerFaction);
		unitGUI.open(gui);
		planetGUI.open(gui, state, playerFaction);
		timescaleGUI.open(gui);
		resourceGUI.open(gui);
		shipDesignerGUI.open(gui, playerFaction);
		announcerGUI.open(gui);
	}
	else {
		unitGUI.open(gui);
		planetGUI.open(gui, state, playerFaction);
		timescaleGUI.open(gui);
	}
}

void DebugConsole::open(tgui::Gui& gui) {
	m_console = tgui::Group::create();
	m_console->getRenderer()->setOpacity(0.75f);
	
	m_chatBox = tgui::ChatBox::create();
	m_chatBox->setSize("33%", "25%");
	m_chatBox->setTextColor(tgui::Color::White);
	m_console->add(m_chatBox);

	m_editBox = tgui::EditBox::create();
	m_editBox->setPosition("0%", "25%");
	m_editBox->setSize("33%", "5%");
	m_editBox->onReturnKeyPress([this]() {
		m_chatBox->addLine(">>> " + m_editBox->getText());
		processCommand(m_editBox->getText().toStdString());
		m_editBox->setText("");
	});
	m_console->add(m_editBox);

	gui.add(m_console);
}

void DebugConsole::close(tgui::Gui& gui) {
	if (m_console != nullptr) {
		gui.remove(m_console);
		m_console = nullptr;
	}
}

void DebugConsole::onEvent(sf::Event& ev, tgui::Gui& gui, GameState& state) {
	if (ev.type == sf::Event::EventType::KeyReleased && ev.key.code == sf::Keyboard::Tilde && state.getState() != GameState::State::MAIN_MENU) {
		if (isOpen()) {
			close(gui);
		}
		else {
			open(gui);
		}
	}
}

void DebugConsole::processCommand(std::string rawCommand) {
	std::stringstream strs(rawCommand);
	std::string word;
	std::vector<std::string> split;

	while (strs >> word) {
		split.push_back(word);
	}

	if (split.size() == 0) return;

	std::string command = split[0];
	std::vector<std::string> args;

	if (split.size() > 1) {
		args.insert(args.begin(), split.begin() + 1, split.end());
	}

	Command c;
	c.command = command;
	c.args = args;

	m_commandQueue.push(c);
}

void DebugConsole::runCommands(Constellation& constellation, GameState& state, sf::RenderWindow& window, tgui::Gui& gui, PlayerGUI& playerGUI) {
	while (m_commandQueue.size() > 0) {
		Command command = m_commandQueue.front();
		m_commandQueue.pop();

		m_chatBox->addLine("Running command " + command.command);

		if (command.command == "spawnship") {
			if (validateArgs(command, 2) && validateState(command, state, GameState::State::LOCAL_VIEW)) {
				std::string type = command.args[0];
				int allegiance = std::atoi(command.args[1].c_str());
				sf::Vector2f pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
				Star* star = state.getLocalViewStar();
				Faction* faction = constellation.getFaction(allegiance);
				sf::Color color = faction->getColor();

				std::unique_ptr<Spaceship> ship = std::make_unique<Spaceship>(type, pos, star, allegiance, color);
				ship->addWeapon(Weapon("LASER_GUN"));
				faction->addSpaceship(star->createSpaceship(ship));

				m_chatBox->addLine("Created spaceship at mouse cursor");
			}
		}
		else if (command.command == "planetdebug") {
			if (validateArgs(command, 0) && validateState(command, state, GameState::State::LOCAL_VIEW)) {
				std::vector<Planet>& planets = state.getLocalViewStar()->getPlanets();
				for (int i = 0; i < planets.size(); i++) {
					m_chatBox->addLine("Planet " + std::to_string(i));
					m_chatBox->addLine("Type: " + planets[i].getTypeString());
					m_chatBox->addLine("Temperature: " + std::to_string(planets[i].getTemperature()));
					m_chatBox->addLine("Atmospheric Pressure: " + std::to_string(planets[i].getAtmosphericPressure()));
					m_chatBox->addLine("Water: " + std::to_string(planets[i].getWater()));
				}
			}
		}
		else if (command.command == "goplanet") {
			if (validateArgs(command, 1) && validateState(command, state, GameState::State::LOCAL_VIEW)) {
				int index = std::atoi(command.args[0].c_str());
				std::vector<Planet>& planets = state.getLocalViewStar()->getPlanets();

				if (index < 0 || index >= planets.size()) {
					m_chatBox->addLine("Invalid index");
				}
				else {
					state.getCamera().setPos(planets[index].getPos());
					m_chatBox->addLine("Set camera pos to planet " + command.args[0]);
				}
			}
		}
		else if (command.command == "mosthabitable") {
			if (validateArgs(command, 0) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				if (state.getState() == GameState::State::LOCAL_VIEW) {
					state.changeToWorldView();
				}
				Star* highestHabitabilityStar = nullptr;
				Planet* highestHabitabilityPlanet = nullptr;
				float highestHabitability = 0.0f;

				for (auto& star : constellation.getStars()) {
					for (auto& planet : star->getPlanets()) {
						float habitability = planet.getHabitability();
						if (habitability > highestHabitability) {
							highestHabitability = habitability;
							highestHabitabilityStar = star.get();
							highestHabitabilityPlanet = &planet;
						}
					}
				}

				if (highestHabitabilityPlanet == nullptr) {
					m_chatBox->addLine("Didn't find a planet somehow");
				}
				else {
					state.changeToLocalView(highestHabitabilityStar);
					state.getCamera().setPos(highestHabitabilityPlanet->getPos());
					state.getCamera().setAbsoluteZoom(1.0f);
					m_chatBox->addLine("Zoomed to the most habitable planet in the constellation");
				}

			}
		}
		else if (command.command == "listfactions") {
			if (validateArgs(command, 0) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				int i = 0;
				for (Faction& faction : constellation.getFactions()) {
					m_chatBox->addLine(std::to_string(i) + ": " + faction.getName());
					i++;
				}
			}
		}
		else if (command.command == "possess") {
			if (validateArgs(command, 1) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				Faction* faction = constellation.getFaction(std::atoi(command.args[0].c_str()));
				if (faction != nullptr) {
					state.getPlayer().setFaction(faction->getID(), faction->getColor());
					state.getPlayer().enableFogOfWar();

					close(gui);

					gui.removeAllWidgets();
					playerGUI.open(gui, state, constellation, false);

					for (auto& star : constellation.getStars()) {
						if (star->getAllShipsOfAllegiance(faction->getID()).size() > 0) {
							star->setDiscovered(true);
						}
						else {
							star->setDiscovered(false);
						}
					}
				}
				else {
					m_chatBox->addLine("Invalid faction");
				}
			}
		}
		else if (command.command == "spectate") {
			if (validateArgs(command, 0) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				state.getPlayer().setFaction(-1, sf::Color(175, 175, 175));
				state.getPlayer().disableFogOfWar();

				close(gui);

				gui.removeAllWidgets();
				playerGUI.open(gui, state, constellation, true);

				constellation.discoverAllStars();
			}
		}		
		else if (command.command == "giveweapon") {
			if (validateArgs(command, 1) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				Faction* playerFaction = constellation.getFaction(state.getPlayer().getFaction());
				if (playerFaction != nullptr) {
					playerFaction->addWeapon(command.args[0]);
				}
			}
		}
		else {
			m_chatBox->addLine("Invalid command " + command.command);
		}
	}
}

bool DebugConsole::validateArgs(const Command& command, int numArgs) {
	if (command.args.size() == numArgs) {
		return true;
	}
	else {
		m_chatBox->addLine("Invalid arguments for command " + command.command);
		return false;
	}
}

bool DebugConsole::validateState(const Command& command, const GameState& state, GameState::State requestedState) {
	if (state.getState() == requestedState) {
		return true;
	}
	else {
		m_chatBox->addLine("Invalid game state for command " + command.command);
		return false;
	}
}

bool DebugConsole::validateNotState(const Command& command, const GameState& state, GameState::State notState) {
	if (state.getState() != notState) {
		return true;
	}
	else {
		m_chatBox->addLine("Invalid game state for command " + command.command);
		return false;
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
				gui.add(m_planetPanel);

				m_planetInfoPanel = tgui::Panel::create();
				m_planetInfoPanel->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
				m_planetInfoPanel->getRenderer()->setBorderColor(tgui::Color::White);
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
				if (state.getLocalViewStar()->isDiscovered()) {
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

	if (playerFaction != nullptr) {
		createBuildingsButton(gui, planet, playerFaction);
	}

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
	}
}

void PlanetGUI::switchSideWindow(const std::string& name, tgui::Gui& gui) {
	if (m_sideWindow == nullptr) {
		m_sideWindow = tgui::ChildWindow::create(name);
		m_sideWindow->setPosition("22.5%", "61%");
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
			resourceListBox->addItem(resource.getTypeString());
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

		auto buildingsBox = tgui::ListBox::create();
		buildingsBox->setPosition("2.5%", "5%");
		buildingsBox->setSize("47.5%", "90%");
		m_sideWindow->add(buildingsBox);

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
						displayBuildingInfo(building, true);
						break;
					}
				}
			}
		});

		if (planet.getColony().getAllegiance() == playerFaction->getID()) {
			auto buildButton = tgui::Button::create("Build");
			buildButton->setPosition("80%", "85%");
			buildButton->onPress([this, &gui, &planet, playerFaction]() {
				switchSideWindow("Build Colony Buildings", gui);

				if (m_sideWindow == nullptr) return;

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
								displayBuildingInfo(building, false);

								auto buildButton = tgui::Button::create("Build");
								buildButton->setPosition("2.5%", "85%");
								m_sideWindow->get<tgui::Group>("infoGroup")->add(buildButton);

								buildButton->onPress([building, &planet]() {
									if (!planet.getColony().hasBuildingOfType(building.getType())) {
										planet.getColony().addBuilding(building);
									}
									});

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

void PlanetGUI::displayBuildingInfo(ColonyBuilding& building, bool status) {
	auto infoGroup = m_sideWindow->get<tgui::Group>("infoGroup");
	if (infoGroup != nullptr) {
		m_sideWindow->remove(infoGroup);
	}

	infoGroup = tgui::Group::create();
	infoGroup->setPosition("50%", "0%");
	infoGroup->setSize("50%", "100%");
	m_sideWindow->add(infoGroup, "infoGroup");

	auto nameLabel = tgui::Label::create(building.getName());
	infoGroup->add(nameLabel);

	auto descriptionLabel = tgui::Label::create(building.getDescription());
	descriptionLabel->setPosition("0%", "10%");
	descriptionLabel->setSize("100%", "50%");
	infoGroup->add(descriptionLabel, "descriptionLabel");

	if (status) {
		auto statusLabel = tgui::Label::create();
		if (!building.isBuilt()) {
			statusLabel->setText("Status: Under Construction");
		}
		else {
			statusLabel->setText("Status: Operational");
		}
		statusLabel->setPosition("descriptionLabel.left", "descriptionLabel.bottom");
		statusLabel->setSize("100%", "25%");
		infoGroup->add(statusLabel);
	}
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
				m_window->setPosition(mouseScreenPos.x, mouseScreenPos.y);
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

void TimescaleGUI::onEvent(sf::Event& ev, tgui::Gui& gui, int& updatesPerSecondTarget) {
	if (m_timescaleLabel != nullptr) {
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

			updatesPerSecondTarget = 60 * m_timescale;
			
			m_timescaleLabel->setText("Timescale: " + std::to_string(m_timescale) + "x");
			m_timescaleLabel->setVisible(true);
			m_timescaleLabel->hideWithEffect(tgui::ShowAnimationType::Fade, tgui::Duration(4000));
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
				auto label = m_resourceGroup->get<tgui::Label>(PlanetResource::getTypeString(resource.first));

				if (label == nullptr) {
					label = tgui::Label::create();
					label->setPosition("0%", (std::to_string(pos) + "%").c_str());
					label->setTextSize(20);
					m_resourceGroup->add(label, PlanetResource::getTypeString(resource.first));
				}

				std::stringstream ss;
				ss << PlanetResource::getTypeString(resource.first) << ": " << std::fixed << std::setprecision(1) << resource.second;

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

void ShipDesignerGUI::open(tgui::Gui& gui, Faction* playerFaction) {
	m_icon.open(gui, tgui::Layout2d("0%", "80%"), tgui::Layout2d("2.5%", "5%"), "data/art/shipicon.png");

	m_icon.panel->onClick([this, &gui, playerFaction]() {
		if (m_window == nullptr) {
			m_window = tgui::ChildWindow::create("Ship Designer");
			m_window->setOrigin(0.5f, 0.5f);
			m_window->setPosition("50%", "50%");
			m_window->setSize("33%", "33%");
			m_window->getRenderer()->setOpacity(0.75f);
			m_window->onClose([this]() {
				m_window = nullptr;
			});

			auto designLabel = tgui::Label::create("Design");
			designLabel->setOrigin(0.5f, 0.5f);
			designLabel->setPosition("12.5%", "2.5%");
			m_window->add(designLabel);

			auto designListBox = tgui::ListBox::create();
			designListBox->setPosition("2.5%", "5%");
			designListBox->setSize("22.5%", "90%");
			designListBox->onItemSelect([this, designListBox, playerFaction]() {
				auto shipChassisListBox = m_window->get<tgui::ListBox>("shipChassisListBox");
				auto shipWeaponsListBox = m_window->get<tgui::ListBox>("shipWeaponsListBox");
				auto designNameTextBox = m_window->get<tgui::EditBox>("designNameTextBox");

				if (designListBox->getSelectedItemIndex() != -1) {
					Spaceship::DesignerShip ship = playerFaction->getShipDesignByName(designListBox->getSelectedItem().toStdString());

					shipChassisListBox->removeAllItems();
					shipChassisListBox->addItem(ship.chassis.name);
					shipChassisListBox->setSelectedItemByIndex(0);

					shipWeaponsListBox->removeAllItems();
					for (auto& weapon : ship.weapons) {
						shipWeaponsListBox->addItem(weapon.name);
					}
					if (shipWeaponsListBox->getItemCount() > 0) {
						shipWeaponsListBox->setSelectedItemByIndex(shipWeaponsListBox->getItemCount() - 1);
					}

					designNameTextBox->setText(ship.name);
					displayShipInfo(playerFaction);
				}
			});
			m_window->add(designListBox, "designListBox");

			auto chassisLabel = tgui::Label::create("Chassis");
			chassisLabel->setOrigin(0.5f, 0.5f);
			chassisLabel->setPosition("38.75%", "2.5%");
			m_window->add(chassisLabel);

			auto chassisListBox = tgui::ListBox::create();
			chassisListBox->setPosition("27.5%", "5%");
			chassisListBox->setSize("22.5%", "30%");
			m_window->add(chassisListBox, "chassisListBox");

			// Add chassis to list box
			for (auto& chassis : playerFaction->getChassis()) {
				chassisListBox->addItem(chassis.name);
			}

			auto weaponsLabel = tgui::Label::create("Weapons");
			weaponsLabel->setOrigin(0.5f, 0.5f);
			weaponsLabel->setPosition("63.75%", "2.5%");
			m_window->add(weaponsLabel);

			auto weaponsListBox = tgui::ListBox::create();
			weaponsListBox->setPosition("52.5%", "5%");
			weaponsListBox->setSize("22.5%", "30%");
			m_window->add(weaponsListBox, "weaponsListBox");

			// Add weapons to list box
			for (auto& weapon : playerFaction->getWeapons()) {
				weaponsListBox->addItem(weapon.name);
			}

			auto shipChassisListBox = tgui::ListBox::create();
			shipChassisListBox->setPosition("27.5%", "50%");
			shipChassisListBox->setSize("22.5%", "30%");
			m_window->add(shipChassisListBox, "shipChassisListBox");

			auto shipWeaponsListBox = tgui::ListBox::create();
			shipWeaponsListBox->setPosition("52.5%", "50%");
			shipWeaponsListBox->setSize("22.5%", "30%");
			m_window->add(shipWeaponsListBox, "shipWeaponsListBox");

			auto designNameLabel = tgui::Label::create("Design Name: ");
			designNameLabel->setOrigin(0.0f, 0.5f);
			designNameLabel->setPosition("27.5%", "90%");
			m_window->add(designNameLabel, "designNameLabel");

			auto designNameTextBox = tgui::EditBox::create();
			designNameTextBox->setPosition("designNameLabel.right", "designNameLabel.top");
			designNameTextBox->setSize("22.5%", "10%");
			m_window->add(designNameTextBox, "designNameTextBox");

			auto designNameSaveButton = tgui::Button::create("Save Design");
			designNameSaveButton->setOrigin(0.0f, 0.5f);
			designNameSaveButton->setPosition("75%", "90%");
			designNameSaveButton->onClick([this, designNameTextBox, shipChassisListBox, shipWeaponsListBox, playerFaction]() {
				if (shipChassisListBox->getItemCount() > 0) {
					if (designNameTextBox->getText().length() > 0 && canChassisFitWeapons(playerFaction)) {
						Spaceship::DesignerShip ship;
						ship.chassis = playerFaction->getChassisByName(shipChassisListBox->getItemByIndex(0).toStdString());
						for (tgui::String& weapon : shipWeaponsListBox->getItems()) {
							ship.weapons.push_back(playerFaction->getWeaponByName(weapon.toStdString()));
						}
						ship.name = designNameTextBox->getText().toStdString();
						playerFaction->addOrReplaceDesignerShip(ship);
						displayShipDesigns(playerFaction);
					}
				}
			});
			m_window->add(designNameSaveButton);

			auto chassisAdderButton = tgui::Button::create("+");
			chassisAdderButton->setOrigin(0.5f, 0.5f);
			chassisAdderButton->setPosition("chassisListBox.left + chassisListBox.width * 0.25", "42.5%");
			chassisAdderButton->onClick([this, playerFaction, chassisListBox, shipChassisListBox]() {
				if (chassisListBox->getSelectedItemIndex() != -1) {
					if (shipChassisListBox->getItemCount() == 0) {
						shipChassisListBox->addItem(chassisListBox->getSelectedItem());
						shipChassisListBox->setSelectedItemByIndex(0);
						displayShipInfo(playerFaction);
					}
				}
				});
			m_window->add(chassisAdderButton, "chassisAdderButton");

			auto chassisRemoverButton = tgui::Button::create("-");
			chassisRemoverButton->setOrigin(0.5f, 0.5f);
			chassisRemoverButton->setPosition("chassisListBox.left + chassisListBox.width * 0.75", "42.5%");
			chassisRemoverButton->setSize("chassisAdderButton.size");
			chassisRemoverButton->onClick([this, playerFaction, shipChassisListBox]() {
				if (shipChassisListBox->getSelectedItemIndex() != -1) {
					shipChassisListBox->removeItemByIndex(shipChassisListBox->getSelectedItemIndex());
					displayShipInfo(playerFaction);
				}
				});
			m_window->add(chassisRemoverButton);

			auto weaponsAdderButton = tgui::Button::create("+");
			weaponsAdderButton->setOrigin(0.5f, 0.5f);
			weaponsAdderButton->setPosition("weaponsListBox.left + weaponsListBox.width * 0.25", "42.5%");
			weaponsAdderButton->onClick([this, playerFaction, weaponsListBox, shipWeaponsListBox, shipChassisListBox]() {
				if (weaponsListBox->getSelectedItemIndex() != -1 && shipChassisListBox->getItemCount() > 0) {
					shipWeaponsListBox->addItem(weaponsListBox->getSelectedItem());
					//if (!canChassisFitWeapons(playerFaction)) {
					//	shipWeaponsListBox->removeItemByIndex(shipWeaponsListBox->getItemCount() - 1);
					//}
					if (shipWeaponsListBox->getItemCount() > 0) {
						shipWeaponsListBox->setSelectedItemByIndex(shipWeaponsListBox->getItemCount() - 1);
					}
					displayShipInfo(playerFaction);
				}
			});
			m_window->add(weaponsAdderButton);

			auto weaponsRemoverButton = tgui::Button::create("-");
			weaponsRemoverButton->setOrigin(0.5f, 0.5f);
			weaponsRemoverButton->setPosition("weaponsListBox.left + weaponsListBox.width * 0.75", "42.5%");
			weaponsRemoverButton->setSize("chassisAdderButton.size");
			weaponsRemoverButton->onClick([this, playerFaction, shipWeaponsListBox]() {
				if (shipWeaponsListBox->getSelectedItemIndex() != -1) {
					shipWeaponsListBox->removeItemByIndex(shipWeaponsListBox->getSelectedItemIndex());
					if (shipWeaponsListBox->getItemCount() > 0) {
						shipWeaponsListBox->setSelectedItemByIndex(shipWeaponsListBox->getItemCount() - 1);
					}
					displayShipInfo(playerFaction);
				}
				});
			m_window->add(weaponsRemoverButton);

			displayShipDesigns(playerFaction);

			gui.add(m_window);
		}
		else {
			gui.remove(m_window);
			m_window = nullptr;
		}
	});
}

void ShipDesignerGUI::displayShipInfo(Faction* playerFaction) {
	auto shipChassisListBox = m_window->get<tgui::ListBox>("shipChassisListBox");
	auto shipWeaponsListBox = m_window->get<tgui::ListBox>("shipWeaponsListBox");
	auto shipInfoGroup = m_window->get<tgui::Group>("shipInfoGroup");

	if (shipChassisListBox->getItemCount() > 0) {
		tgui::String chassisName = shipChassisListBox->getItemByIndex(0);
		std::unordered_map<PlanetResource::RESOURCE_TYPE, float> totalResourceCost;

		Spaceship::DesignerChassis chassis = playerFaction->getChassisByName(chassisName.toStdString());
		for (auto& resource : chassis.resourceCost) {
			totalResourceCost[resource.first] += resource.second;
		}

		float totalWeaponPoints = 0.0f;

		for (tgui::String& str : shipWeaponsListBox->getItems()) {
			Spaceship::DesignerWeapon weapon = playerFaction->getWeaponByName(str.toStdString());
			totalWeaponPoints += weapon.weaponPoints;
			for (auto& resource : weapon.resourceCost) {
				totalResourceCost[resource.first] += resource.second;
			}
		}

		if (shipInfoGroup != nullptr) {
			m_window->remove(shipInfoGroup);
		}

		shipInfoGroup = tgui::Group::create();
		shipInfoGroup->setPosition("weaponsListBox.right + 2.5%", "weaponsListBox.top");
		shipInfoGroup->setSize("22.5%", "90%");
		m_window->add(shipInfoGroup, "shipInfoGroup");

		// Use stringstream to set the decimals properly
		std::stringstream ss;
		ss << "WC: " << std::fixed << std::setprecision(1) << totalWeaponPoints << "/" << chassis.maxWeaponCapacity;

		auto capacityLabel = tgui::Label::create(ss.str());
		capacityLabel->setPosition("0%", "0%");
		tgui::ToolTip::setInitialDelay(0);
		capacityLabel->setToolTip(tgui::Label::create("Weapon Capacity"));
		shipInfoGroup->add(capacityLabel, "capacityLabel");

		ShipDesignerGUI::displayShipResourceCost(shipInfoGroup, totalResourceCost, 10);
	}
	else {
		if (shipInfoGroup != nullptr) {
			m_window->remove(shipInfoGroup);
		}
	}
}

bool ShipDesignerGUI::canChassisFitWeapons(Faction* playerFaction) {
	auto shipChassisListBox = m_window->get<tgui::ListBox>("shipChassisListBox");
	auto shipWeaponsListBox = m_window->get<tgui::ListBox>("shipWeaponsListBox");
	
	Spaceship::DesignerChassis chassis = playerFaction->getChassisByName(shipChassisListBox->getItemByIndex(0).toStdString());

	float total = 0.0f;
	for (tgui::String& weaponString : shipWeaponsListBox->getItems()) {
		Spaceship::DesignerWeapon weapon = playerFaction->getWeaponByName(weaponString.toStdString());
		total += weapon.weaponPoints;
	}

	if (total <= chassis.maxWeaponCapacity) return true;
	else return false;
}

void ShipDesignerGUI::displayShipDesigns(Faction* playerFaction) {
	auto designListBox = m_window->get<tgui::ListBox>("designListBox");
	designListBox->removeAllItems();

	for (Spaceship::DesignerShip& ship : playerFaction->getShipDesigns()) {
		designListBox->addItem(ship.name);
	}
}

void ShipDesignerGUI::displayShipResourceCost(tgui::Group::Ptr group, const std::unordered_map<PlanetResource::RESOURCE_TYPE, float>& totalResourceCost, int yPosPercent) {
	auto resourcesLabel = tgui::Label::create("Resources: ");
	resourcesLabel->setPosition("0%", (std::to_string(yPosPercent) + "%").c_str());
	group->add(resourcesLabel);

	yPosPercent += 10;

	// Add resource cost labels
	for (auto& resource : totalResourceCost) {
		std::stringstream labelString;
		labelString << PlanetResource::getTypeString(resource.first) << ": " << std::fixed << std::setprecision(1) << resource.second;

		auto label = tgui::Label::create(labelString.str());
		label->setPosition("0%", (std::to_string(yPosPercent) + "%").c_str());
		group->add(label);

		yPosPercent += 10;
	}
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

		if ((currentStar->isDrawingHidden() && currentStar->isDiscovered()) || playerAllegiance == -1) {
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