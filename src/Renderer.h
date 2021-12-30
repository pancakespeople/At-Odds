#pragma once
#include "EffectsEmitter.h"
#include "Background.h"

class Renderer : public sf::RenderTexture {
public:
	EffectsEmitter effects;
	Background background;

	Renderer(sf::Vector2i resolution);
	void setResolution(sf::Vector2i resolution);
	void onEvent(const sf::Event& ev);
	void displayToWindow(sf::RenderWindow& window);

private:
	sf::Sprite m_sprite;
};