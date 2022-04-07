#pragma once
#include <memory>
#include <SFML/Graphics.hpp>

#include "Camera.h"
#include "Player.h"

class Star;
class Constellation;

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

	const static int DEFAULT_UPDATE_TARGET = 80;

	GameState(Camera camera);
	
	State getState() const { return m_state; }
	MetaState getMetaState() const { return m_metaState; }
	Star* getLocalViewStar() { return m_localViewStar; }
	Camera& getCamera() { return m_camera; }
	Player& getPlayer() { return m_player; }

	void changeToLocalView(Star* star);
	void changeToWorldView();
	void switchLocalViews(Star* star);
	void onEvent(sf::Event ev);
	void exitGame();
	void loadGame() { m_metaState = MetaState::LOAD_GAME; }
	void resetMetaState() { m_metaState = MetaState::NONE; }
	void addOnChangeStateCallback(std::function<void()> func) { m_changeStateCallbacks.push_back(func); }
	void clearCallbacks() { m_changeStateCallbacks.clear(); }
	void reinitAfterLoad(Constellation& constellation);
	void setTimescale(float timescale);
	void restartUpdateClock() { m_updateClock.restart(); }
	void setArenaGame(bool enabled) { m_arenaGame = enabled; }

	float getTimescale() { return m_timescale; }
	int getUpdatesPerSecondTarget() { return m_updatesPerSecondTarget; }

	bool isArenaGameEnabled() const { return m_arenaGame; }

	const sf::Clock& getUpdateClock() { return m_updateClock; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_state;
		archive & m_localViewStarID;
		archive & m_camera;
		archive & m_player;
		archive & m_timescale;
		archive & m_updatesPerSecondTarget;
	}
	
	void callOnChangeStateCallbacks();

	State m_state = GameState::State::MAIN_MENU;
	MetaState m_metaState = GameState::MetaState::NONE;
	Star* m_localViewStar = nullptr;
	Camera m_camera;
	Player m_player;

	std::vector<std::function<void()>> m_changeStateCallbacks;

	int m_localViewStarID = 0;
	int m_updatesPerSecondTarget = DEFAULT_UPDATE_TARGET;
	float m_timescale = 1.0f;
	bool m_arenaGame = true;
	sf::Clock m_updateClock;
};

