#include "gamepch.h"

#include "GameState.h"
#include "Star.h"
#include "Debug.h"
#include "Camera.h"

void GameState::changeToLocalView(Star* star) {
	if (m_state == GameState::State::LOCAL_VIEW) return;
	
	m_camera.setPos(star->getLocalViewCenter());
	m_camera.setAbsoluteZoom(10.0f);

	star->m_localViewActive = true;
	m_localViewStar = star;
	m_state = GameState::State::LOCAL_VIEW;
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
	m_state = GameState::State::WORLD_VIEW;
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

