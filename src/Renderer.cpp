#include "gamepch.h"
#include "Renderer.h"
#include "GameState.h"

Renderer::Renderer(sf::Vector2i resolution) :
effects(resolution, *this),
background("data/art/spacebackground1.png", resolution.x, resolution.y) {
	m_texture.create(resolution.x, resolution.y);
	m_sprite.setTexture(m_texture.getTexture());
}

void Renderer::setResolution(sf::Vector2i resolution) {
	assert(m_texture.create(resolution.x, resolution.y));
	m_sprite = sf::Sprite(m_texture.getTexture());

	float nebulaSeed = background.getNebulaSeed();
	background = Background(background.getTexturePath(), resolution.x, resolution.y);
	background.setNebulaSeed(nebulaSeed);

	effects.init(resolution);
}

void Renderer::onEvent(const sf::Event& ev) {
	effects.onEvent(ev);
}

void Renderer::displayToWindow(sf::RenderWindow& window, GameState& state) {
	m_texture.display();
	
	effects.drawPostEffects(m_sprite, window, state);

	//window.draw(m_sprite);
}

void Renderer::draw(const sf::Drawable& drawable) {
	m_texture.draw(drawable, sf::RenderStates::Default);
}

void Renderer::draw(const sf::Drawable& drawable, const sf::RenderStates& states) {
	m_texture.draw(drawable, states);
}