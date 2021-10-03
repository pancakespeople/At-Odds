#pragma once
#include <SFML/Graphics.hpp>

class Planet;
class Star;

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
	void drawPlanet(sf::RenderWindow& window, const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed, float time);
	void drawTerraPlanet(sf::RenderWindow& window, const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed, float time);
	void drawGlow(sf::RenderWindow& window, const sf::Vector2f& pos, float size, const sf::Color& color);
	void drawHabitableZone(sf::RenderWindow& window, const sf::Vector2f& starPos, float temperature);
	void drawNebula(sf::RenderWindow& window, sf::Sprite& sprite, float seed);
	void drawSelection(sf::RenderWindow& window, const sf::CircleShape& shape);
	void drawBorders(sf::RenderWindow& window, const sf::RectangleShape& shape, const std::vector<sf::Glsl::Vec2>& points, sf::Color color);
	void updateTime(float time);

private:
	void initShaders(sf::Vector2i resolution);
	
	sf::CircleShape m_jumpBubble;
	sf::CircleShape m_habitableZone;

	sf::RectangleShape m_line;
	sf::RectangleShape m_fogOfWar;
	sf::RectangleShape m_starLocalView;
	sf::RectangleShape m_glow;
	
	sf::Shader m_distanceShader;
	sf::Shader m_starLocalViewShader;
	sf::Shader m_planetShader;
	sf::Shader m_glowShader;
	sf::Shader m_nebulaShader;
	sf::Shader m_selectionShader;
	sf::Shader m_borderShader;
	sf::Shader m_terraPlanetShader;

	sf::Vector2i m_resolution;
};

