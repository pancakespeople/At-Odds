#pragma once
#include <SFML/Graphics.hpp>

class Renderer;
class Camera;

class Background {
public:
	Background(std::string texturePath, int w, int h);

	void draw(Renderer& renderer, Camera& camera);

	std::string getTexturePath() const { return m_texturePath; }

	float getNebulaSeed() const { return m_nebulaSeed; }
	void setNebulaSeed(float seed) { m_nebulaSeed = seed; }
private:
	struct ParallaxAsteroid {
		float zoomLevel = 0.0f;
		sf::Vector2f pos;
	};
	
	sf::Sprite m_sprite;
	std::string m_texturePath;
	std::vector<ParallaxAsteroid> m_asteroids;

	float m_nebulaSeed = 1.0f;
};

