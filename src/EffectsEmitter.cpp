#include "gamepch.h"

#include "EffectsEmitter.h"
#include "Math.h"

EffectsEmitter::EffectsEmitter(sf::Vector2i resolution) {
	init(resolution);
}

void EffectsEmitter::init(sf::Vector2i resolution) {
	m_jumpBubble.setRadius(100.0f);
	m_jumpBubble.setFillColor(sf::Color(255, 0, 255, 125));
	m_jumpBubble.setOrigin(sf::Vector2f(100.0f, 100.0f));

	m_distanceShader.loadFromFile("data/shaders/vertexshader.shader", "data/shaders/distfragshader.shader");
	m_distanceShader.setUniform("resolution", sf::Glsl::Vec2(resolution.x, resolution.y));

	m_fogOfWar.setFillColor(sf::Color(100, 100, 100, 100));
	m_fogOfWar.setPosition(0.0f, 0.0f);
	m_fogOfWar.setSize(sf::Vector2f(resolution.x, resolution.y));
}

void EffectsEmitter::drawJumpBubble(sf::RenderWindow& window, const sf::Vector2f& pos, float radius, float percent) {
	m_jumpBubble.setRadius(radius);
	m_jumpBubble.setOrigin(sf::Vector2f(radius, radius));
	m_jumpBubble.setFillColor(sf::Color(255.0f, 0.0f, 255.0f, 200.0f * (percent / 100.0f)));
	m_jumpBubble.setPosition(pos);
	window.draw(m_jumpBubble);
}

void EffectsEmitter::drawLine(sf::RenderWindow& window, const sf::Vector2f& begin, const sf::Vector2f& end, const sf::Color& color) {
	float angle = Math::angleBetween(begin, end);
	float dist = Math::distance(begin, end);

	m_line.setRotation(-angle);
	m_line.setSize(sf::Vector2f(dist, 5.0f));
	m_line.setFillColor(color);
	m_line.setPosition(begin);

	window.draw(m_line);
}

void EffectsEmitter::drawWithDistanceShader(sf::RenderWindow& window, sf::Shape& s, const sf::Vector2i& pos) {
	window.draw(s, &m_distanceShader);
}

void EffectsEmitter::drawFogOfWar(sf::RenderWindow& window) {
	sf::View oldView = window.getView();
	window.setView(window.getDefaultView());

	window.draw(m_fogOfWar);

	window.setView(oldView);
}