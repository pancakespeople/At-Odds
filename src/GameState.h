#pragma once
#include <memory>
#include <SFML/Graphics.hpp>

#include "Camera.h"
#include "Player.h"

class Star;

class GameState {
public:
	enum class State {
		WORLD_VIEW,
		LOCAL_VIEW,
		MAIN_MENU,
	};
	enum class MetaState {
		NONE,
		EXITING,
		EXIT_AND_SAVE,
		LOAD_GAME,
		SAVE_GAME
	};

	GameState(Camera camera);
	
	State getState() const { return m_state; }

	MetaState getMetaState() const { return m_metaState; }

	void changeToLocalView(Star* star);

	void changeToWorldView();

	Star* getLocalViewStar() { return m_localViewStar; }

	void onEvent(sf::Event ev);

	Camera& getCamera() { return m_camera; }

	void exitGame();

	Player& getPlayer() { return m_player; }

	void loadGame() { m_metaState = MetaState::LOAD_GAME; }

	void resetMetaState() { m_metaState = MetaState::NONE; }
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& m_state;
		archive& m_localViewStar;
		archive& m_camera;
		archive& m_player;
	}
	
	State m_state = GameState::State::MAIN_MENU;
	MetaState m_metaState = GameState::MetaState::NONE;
	Star* m_localViewStar = nullptr;
	Camera m_camera;
	Player m_player;
};

