#include "gamepch.h"

#include "GameState.h"
#include "Star.h"
#include "Debug.h"
#include "Camera.h"
#include "Constellation.h"

void GameState::changeToLocalView(Star* star) {
	if (m_state == GameState::State::LOCAL_VIEW) return;
	
	m_camera.setPos(star->getLocalViewCenter());
	m_camera.setAbsoluteZoom(10.0f);

	star->m_localViewActive = true;
	m_localViewStar = star;
	m_localViewStarID = star->getID();
	m_state = GameState::State::LOCAL_VIEW;

	callOnChangeStateCallbacks();
}

void GameState::changeToWorldView() {
	if (m_state == GameState::State::WORLD_VIEW) return;
	
	if (m_localViewStar == nullptr) {
		m_state = GameState::State::WORLD_VIEW;
		return;
	}

	m_camera.setPos(m_localViewStar->getCenter());
	m_camera.resetZoom();
	
	m_localViewStar->m_localViewActive = false;
	m_localViewStar->clearAnimations();
	m_localViewStar = nullptr;
	m_localViewStarID = 0;
	m_state = GameState::State::WORLD_VIEW;

	callOnChangeStateCallbacks();
}

void GameState::switchLocalViews(Star* star) {
	if (m_state != GameState::State::LOCAL_VIEW) {
		assert(false);
		return;
	}

	m_camera.setPos(star->getLocalViewCenter());
	m_camera.setAbsoluteZoom(10.0f);

	m_localViewStar->m_localViewActive = false;
	m_localViewStar->clearAnimations();

	star->m_localViewActive = true;
	m_localViewStar = star;
	m_localViewStarID = star->getID();
	m_state = GameState::State::LOCAL_VIEW;
}

void GameState::onEvent(sf::Event ev) {
	if (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Tab &&
		m_state == GameState::State::LOCAL_VIEW) {
		changeToWorldView();
	}
}

GameState::GameState(Camera camera) {
	m_camera = camera;
}

void GameState::exitGame() {
	if (m_state == State::MAIN_MENU) {
		// Do not save if on main menu
		m_metaState = MetaState::EXITING;
	}
	else {
		m_metaState = MetaState::EXIT_AND_SAVE;
	}
}

void GameState::callOnChangeStateCallbacks() {
	for (auto& func : m_changeStateCallbacks) {
		func();
	}
}

void GameState::reinitAfterLoad(Constellation& constellation) {
	if (m_localViewStarID == 0) {
		m_localViewStar = nullptr;
	}
	else {
		m_localViewStar = constellation.getStarByID(m_localViewStarID);
	}
}

void GameState::setTimescale(float timescale) {
	m_timescale = timescale;
	m_updatesPerSecondTarget = 80 * m_timescale;
}