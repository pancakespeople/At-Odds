#include "gamepch.h"
#include "Player.h"
#include "Camera.h"
#include "GameState.h"
#include "Spaceship.h"

void Player::setFaction(int allegiance, sf::Color color) {
	m_factionID = allegiance;
	m_color = color;
}

void Player::update(GameState& state) {
	// Controlling a ship
	if (m_controlledShip != nullptr) {
		if (m_controlledShip->isDead() || state.getState() != GameState::State::LOCAL_VIEW) {
			m_controlledShip = nullptr;
		}
		else {
			state.getCamera().setPos(m_controlledShip->getPos());
			state.getCamera().setAllowMovement(false);
			m_controlledShip->setAIEnabled(false);

			if (m_controlledShip->getCurrentStar() != state.getLocalViewStar()) {
				state.switchLocalViews(m_controlledShip->getCurrentStar());
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
				m_controlledShip->accelerate(m_controlledShip->getMaxAcceleration());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
				m_controlledShip->rotateTo(m_controlledShip->getFacingAngle() + 90.0f);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
				m_controlledShip->rotateTo(m_controlledShip->getFacingAngle() - 90.0f);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
				// Fire controlled ships weapons

				for (Weapon& weapon : m_controlledShip->getWeapons()) {
					weapon.fireAtAngle(m_controlledShip->getPos(), m_controlledShip->getAllegiance(), m_controlledShip->getFacingAngle(), m_controlledShip->getCurrentStar());
				}
			}
		}
	}
	else {
		state.getCamera().setAllowMovement(true);
	}
}

void Player::setControlledShip(Spaceship* ship) {
	if (m_controlledShip != nullptr) {
		m_controlledShip->setAIEnabled(true);
	}

	m_controlledShip = ship;
}