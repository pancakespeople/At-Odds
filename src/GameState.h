#pragma once
#include <memory>
#include <SFML/Graphics.hpp>

#include "Camera.h"
#include "Player.h"
#include "GUI.h"

class Star;

class GameState {
public:
	enum class State {
		WORLD_VIEW,
		LOCAL_VIEW,
		EXITING,
		MAIN_MENU
	};

	GameState(Camera camera);
	
	State getState() { return m_state; }

	void changeToLocalView(Star* star);

	void changeToWorldView();

	Star* getLocalViewStar() { return m_localViewStar; }

	void onEvent(sf::Event ev);

	Camera& getCamera() { return m_camera; }

	void exitGame() { m_state = State::EXITING; }

	Player& getPlayer() { return m_player; }
private:
	State m_state = GameState::State::MAIN_MENU;
	Star* m_localViewStar = nullptr;
	Camera m_camera;
	Player m_player;
};

