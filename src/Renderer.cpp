#include "gamepch.h"
#include "Renderer.h"
#include "GameState.h"
#include "Random.h"

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

void Renderer::onEvent(const sf::Event& ev, const sf::RenderWindow& window) {
	effects.onEvent(ev);

	if (ev.type == sf::Event::KeyPressed) {
		if (ev.key.code == sf::Keyboard::F3) {
			sf::Image img = window.capture();
			img.saveToFile("data/screenshots/" + Random::randString(8) + ".png");
		}
	}
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

sf::Vector2f Renderer::worldToScreenPos(sf::Vector2f pos) {
	sf::Vector2f viewPos = getView().getCenter() - pos;
	sf::Vector2f viewSize = getView().getSize();
	sf::Vector2f viewPosNorm = sf::Vector2f(1.0f - (viewPos.x + viewSize.x / 2.0f) / viewSize.x, (viewPos.y + viewSize.y / 2.0f) / viewSize.y);
	return viewPosNorm;
}