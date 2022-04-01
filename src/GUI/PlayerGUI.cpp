#include "gamepch.h"
#include "PlayerGUI.h"
#include "../Constellation.h"

void PlayerGUI::open(tgui::Gui& gui, GameState& state, Constellation& constellation, PlayerGUIState guiState) {
	// An invisible bottom level panel to help with gui focusing
	mainPanel = tgui::Panel::create();
	mainPanel->getRenderer()->setOpacity(0.0f);
	gui.add(mainPanel);

	Faction* playerFaction = constellation.getFaction(state.getPlayer().getFaction());

	if (guiState == PlayerGUIState::PLAYER) {
#ifdef NDEBUG
		helpWindow.open(gui);
#endif
		buildGUI.open(gui, playerFaction);
		unitGUI.open(gui, state.getPlayer());
		planetGUI.open(gui, state, playerFaction, constellation);
		timescaleGUI.open(gui, state);
		resourceGUI.open(gui);
		shipDesignerGUI.open(gui, playerFaction);
		announcerGUI.open(gui);
		colonyListGUI.open(gui, state, constellation);
		techGUI.open(gui, playerFaction);
		newsGUI.open(gui);
	}
	else if (guiState == PlayerGUIState::SPECTATOR) {
		unitGUI.open(gui, state.getPlayer());
		planetGUI.open(gui, state, playerFaction, constellation);
		timescaleGUI.open(gui, state);
	}

	m_state = guiState;
}

void PlayerGUI::update(sf::RenderWindow& window, Renderer& renderer, GameState& state, Constellation& constellation, tgui::Gui& gui) {
	if (m_state != PlayerGUIState::CLOSED) {
		Faction* playerFaction = constellation.getFaction(state.getPlayer().getFaction());

		unitGUI.update(window, renderer, state.getLocalViewStar(), state.getPlayer(), mainPanel, minimapGUI);
		resourceGUI.update(constellation, state.getPlayer(), state.getLocalViewStar());
		announcerGUI.update(gui, playerFaction);
		buildingGUI.update();
		techGUI.update(playerFaction);
		minimapGUI.update(window, state, unitGUI);
		newsGUI.update(playerFaction);
		planetGUI.update();
	}
}

void PlayerGUI::updateSync(sf::RenderWindow& window, GameState& state, Constellation& constellation, tgui::Gui& gui) {
	if (m_state != PlayerGUIState::CLOSED) {
		planetGUI.updateSync(state);
	}
}

void PlayerGUI::onEvent(const sf::Event& ev, tgui::Gui& gui) {
	if (m_state != PlayerGUIState::CLOSED) {
		if (ev.type == sf::Event::KeyReleased) {
			if (ev.key.code == sf::Keyboard::F2) {
				setVisible(gui, !m_visible);
			}
		}
	}

	minimapGUI.onEvent(ev);
}

void PlayerGUI::setVisible(tgui::Gui& gui, bool visible) {
	if (visible) {
		gui.setRelativeViewport(tgui::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
		m_visible = true;
	}
	else {
		gui.setRelativeViewport(tgui::FloatRect(0.0f, 0.0f, 0.0f, 0.0f));
		m_visible = false;
	}
}

void PlayerGUI::draw(sf::RenderWindow& window, Renderer& renderer, GameState& state, Constellation& constellation, Player& player) {
	if (m_state != PlayerGUIState::CLOSED) {
		buildGUI.draw(window, renderer, state.getLocalViewStar(), constellation.getFaction(state.getPlayer().getFaction()));
		unitGUI.draw(window);
		minimapGUI.draw(window, state.getLocalViewStar(), player.getFaction(), state.getCamera(), constellation.getAlliances());
		shipDesignerGUI.draw();
		planetGUI.draw(renderer, window);
	}
}