#include "gamepch.h"
#include "Renderer.h"
#include "GameState.h"

Renderer::Renderer(sf::Vector2i resolution) :
effects(resolution, *this),
background("data/art/spacebackground1.png", resolution.x, resolution.y) {
	create(resolution.x, resolution.y);
	m_sprite.setTexture(getTexture());
}

void Renderer::setResolution(sf::Vector2i resolution) {
	assert(create(resolution.x, resolution.y));
	m_sprite = sf::Sprite(getTexture());

	float nebulaSeed = background.getNebulaSeed();
	background = Background(background.getTexturePath(), resolution.x, resolution.y);
	background.setNebulaSeed(nebulaSeed);

	effects.init(resolution);
}

void Renderer::onEvent(const sf::Event& ev) {
	effects.onEvent(ev);
}

void Renderer::displayToWindow(sf::RenderWindow& window, GameState& state) {
	display();
	
	effects.drawPostEffects(m_sprite, window, state);

	//window.draw(m_sprite);
}