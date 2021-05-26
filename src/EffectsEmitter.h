#pragma once
#include <SFML/Graphics.hpp>

class EffectsEmitter {
public:
	EffectsEmitter(sf::Vector2i resolution);

	void init(sf::Vector2i resolution);
	void onEvent(const sf::Event& event);
	void drawJumpBubble(sf::RenderWindow& window, const sf::Vector2f& pos, float radius, float percent);
	void drawLine(sf::RenderWindow& window, const sf::Vector2f& begin, const sf::Vector2f& end, const sf::Color& color);
	void drawWithDistanceShader(sf::RenderWindow& window, sf::Shape& s, const sf::Vector2i& pos);
	void drawFogOfWar(sf::RenderWindow& window);
	void drawLocalStar(sf::RenderWindow& window, const sf::Sprite& starShape, float time, float seed);

private:
	void initShaders(sf::Vector2i resolution);
	
	sf::CircleShape m_jumpBubble;
	sf::RectangleShape m_line;
	sf::RectangleShape m_fogOfWar;
	sf::RectangleShape m_starLocalView;
	
	sf::Shader m_distanceShader;
	sf::Shader m_starLocalViewShader;

	sf::Vector2i m_resolution;
};

