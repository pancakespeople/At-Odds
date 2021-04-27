#pragma once
#include <SFML/Graphics.hpp>

class Background {
public:
	Background(std::string texturePath, int w, int h);

	void draw(sf::RenderWindow& window);

	void draw(sf::RenderWindow& window, sf::Shader& shader);

	std::string getTexturePath() { return m_texturePath; }
private:
	sf::Sprite m_sprite;
	std::string m_texturePath;
};

