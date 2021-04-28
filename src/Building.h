#pragma once
#include <SFML/Graphics.hpp>

class Building {
public:
	enum class BUILDING_TYPE {
		OUTPOST
	};

	Building(BUILDING_TYPE type, sf::Vector2f pos, int allegiance, sf::Color color);

	void draw(sf::RenderWindow& window);

	void update();

private:
	sf::Sprite m_sprite;
	sf::CircleShape m_collider;
	int m_allegiance;
};

