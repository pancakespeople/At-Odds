#pragma once
#include <SFML/Graphics.hpp>

class EffectsEmitter;

class Background {
public:
	Background(std::string texturePath, int w, int h);

	void draw(sf::RenderWindow& window, EffectsEmitter& emitter);

	std::string getTexturePath() const { return m_texturePath; }

	float getNebulaSeed() const { return m_nebulaSeed; }
	void setNebulaSeed(float seed) { m_nebulaSeed = seed; }
private:
	sf::Sprite m_sprite;
	std::string m_texturePath;

	float m_nebulaSeed = 1.0f;
};

