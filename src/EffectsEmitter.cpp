#include "gamepch.h"

#include "EffectsEmitter.h"
#include "Math.h"

EffectsEmitter::EffectsEmitter(sf::Vector2i resolution) {
	init(resolution);
}

void EffectsEmitter::init(sf::Vector2i resolution) {
	initShaders(resolution);
	
	m_jumpBubble.setRadius(100.0f);
	m_jumpBubble.setFillColor(sf::Color(255, 0, 255, 125));
	m_jumpBubble.setOrigin(sf::Vector2f(100.0f, 100.0f));

	m_fogOfWar.setFillColor(sf::Color(100, 100, 100, 100));
	m_fogOfWar.setPosition(0.0f, 0.0f);
	m_fogOfWar.setSize(sf::Vector2f(resolution.x, resolution.y));

	m_starLocalView.setSize(sf::Vector2f(2048.0f, 2048.0f));
	m_starLocalView.setOrigin(1024.0f, 1024.0f);

	m_resolution = resolution;
}

void EffectsEmitter::initShaders(sf::Vector2i resolution) {
	m_distanceShader.loadFromFile("data/shaders/vertexshader.shader", "data/shaders/distfragshader.shader");
	m_distanceShader.setUniform("resolution", sf::Glsl::Vec2(resolution.x, resolution.y));
	m_starLocalViewShader.loadFromFile("data/shaders/vertexshader.shader", "data/shaders/starlocalview.shader");
	m_starLocalViewShader.setUniform("size", sf::Glsl::Vec2(2048, 2048));
}

void EffectsEmitter::onEvent(const sf::Event& event) {
	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::F1) {
			initShaders(m_resolution);
			DEBUG_PRINT("Reloaded shaders");
		}
	}
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

void EffectsEmitter::drawLocalStar(sf::RenderWindow& window, const sf::Sprite& starSprite, float time) {
	m_starLocalView.setFillColor(sf::Color::Red);
	m_starLocalView.setPosition(starSprite.getPosition());

	m_starLocalViewShader.setUniform("time", time);

	window.draw(m_starLocalView, &m_starLocalViewShader);
}
