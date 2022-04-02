#pragma once
#include <SFML/Graphics.hpp>

class Planet;
class Star;
class Renderer;
class GameState;
class Camera;

class EffectsEmitter {
public:
	EffectsEmitter(sf::Vector2i resolution, Renderer& renderer);

	void init(sf::Vector2i resolution);
	void onEvent(const sf::Event& event);
	void drawJumpBubble(const sf::Vector2f& pos, float radius, float percent);
	void drawLine(const sf::Vector2f& begin, const sf::Vector2f& end, const sf::Color& color);
	void drawWithDistanceShader(sf::Shape& s, const sf::Vector2i& pos);
	void drawFogOfWar();
	void drawMapStar(const sf::CircleShape& shape, bool flashing);
	void drawLocalStar(const sf::RectangleShape& starRect, float time, float seed);
	void drawPlanet(const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed, float time);
	void drawTerraPlanet(const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed, float time);
	void drawTerraPlanet(const sf::RectangleShape& shape, float planetRadius, sf::Vector2f planetPos, sf::Vector2f sunPos, float seed, float time);
	void drawLavaPlanet(const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed);
	void drawGlow(const sf::Vector2f& pos, float size, const sf::Color& color);
	void drawHabitableZone(sf::RenderWindow& window, const sf::Vector2f& starPos, float temperature);
	void drawNebula(sf::Sprite& sprite, float seed);
	void drawSelection(sf::Vector2f pos, float radius);
	void drawBorders(const sf::RectangleShape& shape, const std::vector<sf::Glsl::Vec3>& points, sf::Color color);
	void drawBlackHole(const sf::RectangleShape& starRect, float time, float seed);
	void drawRings(sf::Vector2f pos, float radius, float seed);
	void drawAsteroidBelt(const std::vector<sf::Vertex>& vertices, float rotationSpeed, sf::Vector2f sunPos);
	void drawPostEffects(sf::Sprite& sprite, sf::RenderWindow& window, GameState& state);
	void drawLaserAnimation(sf::Vector2f sourcePos, sf::Vector2f endPos, float step);
	void drawGatlingAnimation(sf::Vector2f sourcePos, sf::Vector2f endPos, float step);
	void drawBeamAnimation(sf::Vector2f sourcePos, sf::Vector2f endPos, float step, sf::Color color);
	void addExplosionEffect(sf::Vector2f pos, Star* star);
	void drawParallaxBackground(Camera& camera);
	void drawLightningEffect(sf::Vector2f pos, float time, float seed);
	void drawJumpEffect(sf::Vector2f pos, float time, float seed);
	void drawJumpPoint(sf::Vector2f pos, float time, float seed);
	void drawJumpTrail(const sf::RectangleShape& shape);
	void drawParticles(const std::vector<sf::Vertex>& vertices);
	void drawGatlingLaserAnimation(sf::Vector2f sourcePos, sf::Vector2f endPos, float step);
	void drawPlanetMap(tgui::Canvas* canvas, Planet& planet, const sf::RenderWindow& window, bool showPopulation, sf::Vector2i selectedTile);
	void updateTime(float time, float gameTime);

private:
	struct ExplosionEffect {
		sf::Glsl::Vec2 point;
		float time;
		Star* star;
	};
	
	void initShaders(sf::Vector2i resolution);
	
	Renderer& m_renderer;

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
	sf::Shader m_blackHoleShader;
	sf::Shader m_lavaPlanetShader;
	sf::Shader m_mapStarShader;
	sf::Shader m_ringsShader;
	sf::Shader m_asteroidBeltShader;
	sf::Shader m_postEffectsShader;
	sf::Shader m_parallaxShader;
	sf::Shader m_lightningShader;
	sf::Shader m_jumpShader;
	sf::Shader m_jumpPointShader;
	sf::Shader m_jumpTrailShader;
	sf::Shader m_particleShader;
	sf::Shader m_beamShader;
	sf::Shader m_planetMapShader;
	sf::Shader m_terraPlanetMapShader;
	sf::Shader m_lavaPlanetMapShader;

	sf::Vector2i m_resolution;

	std::vector<ExplosionEffect> m_explosionEffects;

	float m_lastGameTime = 0.0f;
	float m_lastTime = 0.0f;

	inline static const std::string m_vertexShaderPath = "data/shaders/vertexshader.shader";
};

