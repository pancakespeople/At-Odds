#include "gamepch.h"
#include "GUI.h"
#include "Spaceship.h"
#include "Star.h"
#include "Order.h"
#include "Pathfinder.h"
#include "Hyperlane.h"
#include "Constellation.h"
#include "GameState.h"
#include "SimpleIni.h"
#include "Background.h"
#include "EffectsEmitter.h"
#include "Sounds.h"
#include "TextureCache.h"

UnitGUI::UnitGUI() {
	m_mouseSelectionBox.setFillColor(sf::Color(150.0f, 150.0f, 150.0f, 100.0f));
}

void UnitGUI::update(const sf::RenderWindow& window, Star* currentStar, int playerFaction) {
	static bool mouseHeld = false;

	m_selectedShips.erase(std::remove_if(m_selectedShips.begin(), m_selectedShips.end(), [](Spaceship* s) {return s->isDead(); }), m_selectedShips.end());

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
		
		m_selectedShips.clear();
		
		if (currentStar != nullptr) {
			for (Spaceship* s : currentStar->getSpaceships()) {
				if (s->getCurrentStar()->isLocalViewActive() && s->getAllegiance() == playerFaction) {

					sf::Vector2i screenPos = window.mapCoordsToPixel(s->getPos());
					sf::FloatRect selection = m_mouseSelectionBox.getGlobalBounds();

					if (screenPos.x >= selection.left && screenPos.x <= selection.left + selection.width &&
						screenPos.y >= selection.top && screenPos.y <= selection.top + selection.height) {
						s->onSelected();
						m_selectedShips.push_back(s);
					}
					else if (s->isSelected()) {
						s->onDeselected();
					}
				}
			}
		}

		m_mouseSelectionBox.setSize(sf::Vector2f(0.0f, 0.0f));
	}
	
	// Mouse click - deselect ships
	else if (mouseHeld && !sf::Mouse::isButtonPressed(sf::Mouse::Left) && m_mouseSelectionBox.getSize().x < 5.0f && m_mouseSelectionBox.getSize().y < 5.0f) {
		for (Spaceship* s : m_selectedShips) {
			if (s->isSelected()) {
				s->onDeselected();
			}
		}
		m_selectedShips.clear();
	}

	mouseHeld = sf::Mouse::isButtonPressed(sf::Mouse::Left);
}

void UnitGUI::draw(sf::RenderWindow& window) {
	window.setView(window.getDefaultView());
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
		window.draw(m_mouseSelectionBox);
	}
}

void UnitGUI::onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state, std::vector<std::unique_ptr<Star>>& stars) {
	if (state.getState() == GameState::State::LOCAL_VIEW) {
		if (ev.type == sf::Event::MouseButtonPressed) {
			if (ev.mouseButton.button == sf::Mouse::Right) {
				
				if (m_selectedShips.size() > 0) {
					sf::Vector2i screenPos = sf::Mouse::getPosition(window);
					sf::Vector2f worldClick = window.mapPixelToCoords(screenPos);
					JumpPoint* jumpPoint = nullptr;
					Spaceship* attackTarget = nullptr;

					// Check if click was on a jump point
					for (JumpPoint& j : state.getLocalViewStar()->getJumpPoints()) {
						if (j.isPointInRadius(worldClick)) {
							jumpPoint = &j;
							break;
						}
					}

					// Check if click was on an enemy ship
					for (Spaceship* s : state.getLocalViewStar()->getSpaceships()) {
						if (s->getAllegiance() != m_selectedShips[0]->getAllegiance()) {
							if (s->getCollider().getGlobalBounds().contains(worldClick)) {
								attackTarget = s;
								break;
							}
						}
					}

					// Add orders
					for (Spaceship* s : m_selectedShips) {
						if (state.getLocalViewStar() == s->getCurrentStar()) {
							s->clearOrders();
							if (attackTarget != nullptr) {
								s->addOrder(AttackOrder(attackTarget));
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
					s->clearOrders();
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
	auto guiWindow = tgui::ChildWindow::create("Main Menu");
	guiWindow->setSize("20%", "40%");
	guiWindow->setPosition("(parent.size - size) / 2");
	guiWindow->setInheritedOpacity(0.75f);
	guiWindow->setTitleButtons(tgui::ChildWindow::TitleButton::None);
	gui.add(guiWindow);
	m_window = guiWindow;

	auto newGameButton = tgui::Button::create("New Game");
	newGameButton->setPosition("(parent.width - width) / 2", "20%");
	newGameButton->connect(tgui::Signals::Button::Pressed, &MainMenu::toNewGameMenu, this, std::ref(gui), std::ref(constellation), std::ref(state));
	guiWindow->add(newGameButton);

	auto loadGameButton = tgui::Button::create("Load Game");
	loadGameButton->setPosition("(parent.width - width) / 2", "40%");
	loadGameButton->onPress.connect([] {DEBUG_PRINT("Not implemented"); });
	guiWindow->add(loadGameButton);

	auto optionsButton = tgui::Button::create("Options");
	optionsButton->setPosition("(parent.width - width) / 2", "60%");
	optionsButton->connect(tgui::Signals::Button::Pressed, &MainMenu::toOptionsMenu, this, std::ref(gui), std::ref(constellation), std::ref(state));
	guiWindow->add(optionsButton);

	auto exitButton = tgui::Button::create("Exit");
	exitButton->setPosition("(parent.width - width) / 2", "80%");
	exitButton->connect(tgui::Signals::Button::Pressed, &MainMenu::exitGame, this, std::ref(state));
	guiWindow->add(exitButton);

	m_opened = true;
}

void MainMenu::close() {
	m_window->close();
	m_opened = false;
}

void MainMenu::exitGame(GameState& state) {
	state.exitGame();
}

void MainMenu::toNewGameMenu(tgui::Gui& gui, Constellation& constellation, GameState& state) {
	close();
	m_newGameMenu.open(gui, constellation, state, this);
}

void MainMenu::toOptionsMenu(tgui::Gui& gui, Constellation& constellation, GameState& state) {
	close();
	m_optionsMenu.open(gui, constellation, state, this);
}

void MainMenu::onEvent(sf::Event& ev, tgui::Gui& gui, Constellation& constellation, GameState& state) {
	if (ev.type == sf::Event::KeyPressed) {
		if (ev.key.code == sf::Keyboard::Escape && state.getState() != GameState::State::MAIN_MENU && !m_opened) {
			m_newGameMenu.close();
			m_optionsMenu.close(gui);
			open(gui, constellation, state);
		}
		else if (ev.key.code == sf::Keyboard::Escape && state.getState() != GameState::State::MAIN_MENU && m_opened) {
			close();
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
	mainMenuButton->connect(tgui::Signals::Button::Pressed, &NewGameMenu::backToMainMenu, this, std::ref(gui), std::ref(constellation), std::ref(state), mainMenu);
	guiWindow->add(mainMenuButton);

	auto startGameButton = tgui::Button::create("Start Game ->");
	startGameButton->setPosition("95% - width", "90%");
	startGameButton->connect(tgui::Signals::Button::Pressed, &NewGameMenu::startNewGame, this, std::ref(gui), std::ref(constellation), std::ref(state));
	guiWindow->add(startGameButton);

	// Stars slider

	auto numStarsLabel = tgui::Label::create("Number of stars");
	numStarsLabel->setPosition("5%", "0%");
	guiWindow->add(numStarsLabel);

	auto numStarsSlider = tgui::Slider::create(20.0f, 500.0f);
	numStarsSlider->setValue(20.0f);
	numStarsSlider->setStep(1.0f);
	numStarsSlider->setPosition("5%", "5%");
	numStarsSlider->connect(tgui::Signals::Slider::ValueChanged, &NewGameMenu::onStarsSliderChange, this, std::ref(gui));
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
	numFactionsSlider->connect(tgui::Signals::Slider::ValueChanged, &NewGameMenu::onFactionsSliderChange, this, std::ref(gui));
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
	m_window->close();
}

void NewGameMenu::backToMainMenu(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu) {
	close();
	mainMenu->open(gui, constellation, state);
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

	state.getCamera().setPos(stars[0]->getPos());
	state.getCamera().resetZoom();

	bool spectate = m_window->get<tgui::CheckBox>("spectateCheckBox")->isChecked();

	gui.removeAllWidgets();

	if (!spectate) {
		constellation.getFactions()[0].controlByPlayer(state.getPlayer());
		state.getCamera().setPos(constellation.getFactions()[0].getCapitol()->getCenter());

		m_playerGui.open(gui);
	}

	state.changeToWorldView();
	close();
}

OptionsMenu::OptionsMenu() {
	CSimpleIniA ini;
	SI_Error rc = ini.LoadFile("data/config/config.ini");
	
	if (rc < 0) {
		DEBUG_PRINT("Error opening config file");
	}

	m_settings.resolution = getKeyIni(ini, "video", "resolution", "1920x1080");
	m_settings.fullscreen = std::stoi(getKeyIni(ini, "video", "fullscreen", "0"));
	m_settings.audioVolume = std::stof(getKeyIni(ini, "audio", "volume", "100").c_str());

	rc = ini.SaveFile("data/config/config.ini");

	if (rc < 0) {
		DEBUG_PRINT("Failed to save config file");
	}
}

void OptionsMenu::saveSettingsToFile() {
	CSimpleIniA ini;
	
	ini.SetValue("video", "resolution", m_settings.resolution.c_str());
	ini.SetValue("video", "fullscreen", std::to_string(m_settings.fullscreen).c_str());
	ini.SetValue("audio", "volume", std::to_string(m_settings.audioVolume).c_str());

	SI_Error rc = ini.SaveFile("data/config/config.ini");

	if (rc < 0) {
		DEBUG_PRINT("Failed to save config file");
	}
}

std::string OptionsMenu::getKeyIni(CSimpleIniA& ini, const char* section, const char* key, const char* defaultValue) {
	const char* resolutionWidth;
	resolutionWidth = ini.GetValue(section, key);
	if (resolutionWidth == nullptr) {
		ini.SetValue(section, key, defaultValue);
		DEBUG_PRINT("Set new default config value: " << section << " " << key << " " << defaultValue);
		return std::string(defaultValue);
	}
	else {
		return std::string(resolutionWidth);
	}
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
	tabs->connect(tgui::Signals::Tabs::TabSelected, &OptionsMenu::onTabChange, this, std::ref(gui));
	guiWindow->add(tabs, "optionsTabs");

	auto group = tgui::Group::create();
	group->setPosition("0%", "optionsTabs.height");
	group->setSize("100%", "100% - optionsTabs.height");
	guiWindow->add(group, "optionsGroup");

	auto mainMenuButton = tgui::Button::create("<- Main Menu");
	mainMenuButton->setPosition("5%", "90%");
	mainMenuButton->connect(tgui::Signals::Button::Pressed, &OptionsMenu::backToMainMenu, this, std::ref(gui), std::ref(constellation), std::ref(state), mainMenu);
	guiWindow->add(mainMenuButton);

	m_group = group;

	onTabChange(gui);
}

void OptionsMenu::close(tgui::Gui& gui) {
	changeSettings(gui);
	saveSettingsToFile();
	if (m_window != nullptr) m_window->close();
}

void OptionsMenu::backToMainMenu(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu) {
	close(gui);
	mainMenu->open(gui, constellation, state);
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
		m_settings.resolution = resolution->getSelectedItem();
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
			background = Background(background.getTexturePath(), res.x, res.y);
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

	std::string helpText = "Welcome to At Odds. "
		"Hold down the middle mouse button to move the camera. "
		"Click on stars in order to go to their local view, and press tab to return to the global view. "
		"At the center of the screen should be your home system. Click on it to view it. \n\n"
		"Click and drag to select your units, right click to order them to move and right click on one of the swirly purple things to order a jump between systems. "
		"You can also order ships to travel to a specified star by selecting them in the local view and clicking on a star in the global view. "
		"Ships will automatically attack enemies unless ordered to move. \n\n"
		"At the moment you just receive a free ship every so often, "
		"and that time period decreases based on how many systems you own. "
		"So you will get more ships if you own more systems. \n\n"
		"Your goal is to defeat all your enemies. Good luck!";

	auto text = tgui::Label::create(helpText);
	text->setSize("100%", "100%");
	window->add(text);
}

void BuildGUI::open(tgui::Gui& gui) {
	auto panel = tgui::Panel::create();
	panel->setSize("2.5%", "5%");
	panel->setPosition("0%", "90%");
	panel->setInheritedOpacity(0.75f);
	gui.add(panel);

	auto picture = tgui::Picture::create(TextureCache::getTexture("data/art/buildicon.png"));
	picture->setSize("100%", "100%");
	picture->setInheritedOpacity(0.75f);
	picture->connect(tgui::Signals::Picture::MouseEntered, &BuildGUI::onMouseEnter, this);
	picture->connect(tgui::Signals::Picture::MouseLeft, &BuildGUI::onMouseExit, this);
	m_buildIcon = picture;
	panel->add(picture);
}

void BuildGUI::onMouseEnter() {
	m_buildIcon->showWithEffect(tgui::ShowAnimationType::Scale, sf::milliseconds(500));
}

void BuildGUI::onMouseExit() {

}

void PlayerGUI::open(tgui::Gui& gui) {
	m_helpWindow.open(gui);
	m_buildGUI.open(gui);
}

