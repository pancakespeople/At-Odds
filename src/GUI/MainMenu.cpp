#include "gamepch.h"
#include "MainMenu.h"

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