#include "gamepch.h"

#include "Background.h"
#include "TextureCache.h"
#include "EffectsEmitter.h"
#include "Random.h"
#include "Renderer.h"
#include "Camera.h"

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
	m_nebulaSeed = Random::randFloat(0.0f, 1.0f);
}

void Background::draw(Renderer& renderer, Camera& camera) {
	renderer.setView(renderer.getDefaultView());
	renderer.effects.drawNebula(m_sprite, m_nebulaSeed);
	renderer.effects.drawParallaxBackground(camera);
}

