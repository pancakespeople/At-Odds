#include "gamepch.h"

#include "Background.h"
#include "TextureCache.h"

#include <iostream>

Background::Background(std::string texturePath, int w, int h) {
	sf::Texture& texture = TextureCache::getTexture(texturePath);
	m_sprite.setTexture(texture);

	float oldWidth = texture.getSize().x;
	float oldHeight = texture.getSize().y;
	
	std::cout << oldWidth << std::endl;
	std::cout << oldHeight << std::endl;
	std::cout << w << std::endl;
	std::cout << h << std::endl;
	
	m_sprite.setTextureRect(sf::IntRect(0, 0, w, h));
	m_sprite.scale(sf::Vector2f(w / oldWidth, h / oldHeight));

	std::cout << "Scale factor x: " << w / oldWidth << std::endl;
	std::cout << "Scale factor y: " << h / oldHeight << std::endl;

	m_texturePath = texturePath;
}

void Background::draw(sf::RenderWindow& window) {
	window.setView(window.getDefaultView());
	window.draw(m_sprite);
}

void Background::draw(sf::RenderWindow& window, sf::Shader& shader) {
	window.setView(window.getDefaultView());
	shader.setUniform("texture", sf::Shader::CurrentTexture);
	window.draw(m_sprite, &shader);
}
