#include "gamepch.h"
#include "Player.h"

void Player::setFaction(int allegiance, sf::Color color) {
	m_factionID = allegiance;
	m_color = color;
}