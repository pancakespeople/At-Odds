#include "gamepch.h"
#include "MainMenu.h"
#include "../Random.h"
#include "../Constellation.h"
#include "../Sounds.h"
#include "../Renderer.h"
#include "../Keybindings.h"

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
	m_title->getRenderer()->setFont("data/fonts/Dirga.ttf");
	m_title->getRenderer()->setTextStyle(tgui::TextStyle::Italic);
	gui.add(m_title);

	auto newGameButton = GUI::Button::create("New Game");
	newGameButton->setPosition("20%", "50%");
	newGameButton->setOrigin(0.5f, 0.5f);
	newGameButton->onPress([this, &gui, &constellation, &state]() {
		close(gui);
		m_newGameMenu.open(gui, constellation, state, this);
		});
	m_panel->add(newGameButton);

	auto loadGameButton = GUI::Button::create("Load Game");
	loadGameButton->setPosition("40%", "50%");
	loadGameButton->setOrigin(0.5f, 0.5f);
	loadGameButton->onPress([&state] {
		state.loadGame();
		});
	m_panel->add(loadGameButton);

	auto optionsButton = GUI::Button::create("Options");
	optionsButton->setPosition("60%", "50%");
	optionsButton->setOrigin(0.5f, 0.5f);
	optionsButton->onPress([this, &gui, &constellation, &state]() {
		close(gui);
		m_optionsMenu.open(gui, constellation, state, this);
		});
	m_panel->add(optionsButton);

	auto exitButton = GUI::Button::create("Exit");
	exitButton->setPosition("80%", "50%");
	exitButton->setOrigin(0.5f, 0.5f);
	exitButton->onPress(&MainMenu::exitGame, this, std::ref(state));
	m_panel->add(exitButton);

	m_opened = true;
	m_starSeed = Random::randFloat(0.0f, 1.0f);

	m_starRect.setPosition(500.0f, 500.0f);
	m_starRect.setSize(sf::Vector2f(2048.0f, 2840.0f));
	m_starRect.setOrigin(sf::Vector2f(2048.0f / 2.0f, 2840.0f / 2.0f));

	int starColor = Random::randInt(0, 3);
	switch (starColor) {
	case 0:
		m_starRect.setFillColor(sf::Color::Red);
		break;
	case 1:
		m_starRect.setFillColor(sf::Color::Yellow);
		break;
	case 2:
		m_starRect.setFillColor(sf::Color(0, 255, 255));
		break;
	case 3:
		m_starRect.setFillColor(sf::Color::White);
		break;
	}

	m_planetRect.setPosition(800.0f, 600.0f);
	m_planetRect.setSize(sf::Vector2f(250.0f, 250.0f));
	m_planetRect.setOrigin(sf::Vector2f(250.0f / 2.0f, 250.0f / 2.0f));
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
		if (!m_forceOpen) {
			if (Keybindings::isKeyPress("MainMenu", ev) && state.getState() != GameState::State::MAIN_MENU && !m_opened) {
				m_newGameMenu.close();
				m_optionsMenu.close(gui);
				open(gui, constellation, state);
			}
			else if (Keybindings::isKeyPress("MainMenu", ev) && state.getState() != GameState::State::MAIN_MENU && m_opened) {
				close(gui);
			}
		}
		else {
			if (Keybindings::isKeyPress("HideUI", ev)) {
				m_panel->setVisible(!m_panel->isVisible());
				m_title->setVisible(!m_title->isVisible());
			}
		}
	}
}

void MainMenu::drawPreview(Renderer& renderer, const GameState& state, float time) {
	if (state.getState() == GameState::State::MAIN_MENU) {
		renderer.effects.drawLocalStar(m_starRect, time, m_starSeed);
		renderer.effects.drawTerraPlanet(m_planetRect, m_planetRect.getSize().x / 2.0f, m_planetRect.getPosition(), m_starRect.getPosition(), m_starSeed, time);
	}
}

void MainMenu::spawnArena(tgui::Gui& gui, Constellation& constellation, GameState& state, PlayerGUI& playerGUI) {
	Camera camera = state.getCamera();

	state = GameState(camera);

	constellation = Constellation();
	constellation.generateOneStarConstellation();
	constellation.setupStars();

	Star* star = constellation.getStars()[0].get();

	state.getCamera().setPos(sf::Vector2f(0.0f, 0.0f));
	state.getCamera().resetZoom();

	gui.removeAllWidgets();
	state.clearCallbacks();
	Sounds::clearSounds();

	state.changeToLocalView(star);

	constellation.onStart();

	playerGUI.open(gui, state, constellation, PlayerGUIState::CLOSED);

	const std::vector<std::string> chassis = { "FRIGATE", "DESTROYER", "CRUISER" };
	for (int i = 0; i < 5; i++) {
		star->generateRandomShip(sf::Vector2f(Random::randFloat(-10000.0f, 0.0f), Random::randFloat(-5000.0f, 5000.0f)), 0, sf::Color::Blue, chassis);
	}

	for (int i = 0; i < 5; i++) {
		star->generateRandomShip(sf::Vector2f(Random::randFloat(0.0f, 10000.0f), Random::randFloat(-5000.0f, 5000.0f)), 1, sf::Color::Red, chassis);
	}
}

void MainMenu::updateArena(int ticks, Constellation& constellation, const GameState& state) {
	if (state.isArenaGameEnabled()) {
		int blues = 0;
		int reds = 0;

		Star* star = constellation.getStars()[0].get();
		auto& ships = star->getSpaceships();

		for (auto& ship : ships) {
			if (ship->getAllegiance() == 0) blues++;
			else reds++;
		}

		if (ticks % 180 == 0) {
			int total = blues + reds;
			if (total < 50) {
				const std::vector<std::string> chassis = { "FRIGATE", "DESTROYER", "CRUISER" };
				if (blues < reds) {
					star->generateRandomShip(sf::Vector2f(Random::randFloat(-10000.0f, 0.0f), Random::randFloat(-5000.0f, 5000.0f)), 0, sf::Color::Blue, chassis);
				}
				else {
					star->generateRandomShip(sf::Vector2f(Random::randFloat(0.0f, 10000.0f), Random::randFloat(-5000.0f, 5000.0f)), 1, sf::Color::Red, chassis);
				}
			}
		}
	}
}