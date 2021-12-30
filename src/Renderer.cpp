#include "gamepch.h"
#include "Renderer.h"

Renderer::Renderer(sf::Vector2i resolution) :
effects(resolution),
background("data/art/spacebackground1.png", resolution.x, resolution.y) {
	create(resolution.x, resolution.y);
	m_sprite.setTexture(getTexture());
}

void Renderer::setResolution(sf::Vector2i resolution) {
	create(resolution.x, resolution.y);
	m_sprite.setTexture(getTexture());

	float nebulaSeed = background.getNebulaSeed();
	background = Background(background.getTexturePath(), resolution.x, resolution.y);
	background.setNebulaSeed(nebulaSeed);

	effects.init(resolution);
}

void Renderer::onEvent(const sf::Event& ev) {
	effects.onEvent(ev);
}

void Renderer::displayToWindow(sf::RenderWindow& window) {
	display();

	window.draw(m_sprite);
}