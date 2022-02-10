#pragma once
#include "EffectsEmitter.h"
#include "Background.h"

class GameState;

class Renderer {
public:
	EffectsEmitter effects;
	Background background;

	Renderer(sf::Vector2i resolution);
	void setResolution(sf::Vector2i resolution);
	void onEvent(const sf::Event& ev, const sf::RenderWindow& window);
	void displayToWindow(sf::RenderWindow& window, GameState& state);
	void draw(const sf::Drawable& drawable);
	void draw(const sf::Drawable& drawable, const sf::RenderStates& states);
	sf::Vector2f worldToScreenPos(sf::Vector2f pos);
	void draw(const sf::Vertex* vertices, std::size_t vertexCount, sf::PrimitiveType type, const sf::RenderStates& states = sf::RenderStates::Default) { m_texture.draw(vertices, vertexCount, type, states); }
	void setView(const sf::View& view) { m_texture.setView(view); }
	void clear() { m_texture.clear(); }
	
	const sf::View& getDefaultView() const { return m_texture.getDefaultView(); }
	const sf::View& getView() const { return m_texture.getView(); }

	sf::Vector2f mapPixelToCoords(sf::Vector2i pixel) const { return m_texture.mapPixelToCoords(pixel); }
	sf::Vector2i mapCoordsToPixel(sf::Vector2f coords) const { return m_texture.mapCoordsToPixel(coords); }

private:
	sf::Sprite m_sprite;
	sf::RenderTexture m_texture;
};