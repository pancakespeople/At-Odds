#pragma once
#include <SFML/Graphics.hpp>

class EffectsEmitter {
public:
	EffectsEmitter(sf::Vector2i resolution);

	void init(sf::Vector2i resolution);
	
	void drawJumpBubble(sf::RenderWindow& window, const sf::Vector2f& pos, float radius, float percent);

	void drawLine(sf::RenderWindow& window, const sf::Vector2f& begin, const sf::Vector2f& end, const sf::Color& color);

	void drawWithDistanceShader(sf::RenderWindow& window, sf::Shape& s, const sf::Vector2i& pos);

	void drawFogOfWar(sf::RenderWindow& window);

private:
	sf::CircleShape m_jumpBubble;
	sf::RectangleShape m_line;
	sf::Shader m_distanceShader;
	sf::RectangleShape m_fogOfWar;
};

