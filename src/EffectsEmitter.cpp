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

	m_planetShader.loadFromFile("data/shaders/vertexshader.shader", "data/shaders/planetshader.shader");
	m_planetShader.setUniform("size", sf::Glsl::Vec2(2048, 2048));

	m_glowShader.loadFromFile("data/shaders/vertexshader.shader", "data/shaders/glow.shader");
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

void EffectsEmitter::drawLocalStar(sf::RenderWindow& window, const sf::Sprite& starSprite, float time, float seed) {
	m_starLocalView.setFillColor(starSprite.getColor());
	m_starLocalView.setPosition(starSprite.getPosition());
	m_starLocalView.setScale(starSprite.getScale());

	m_starLocalViewShader.setUniform("time", time);
	m_starLocalViewShader.setUniform("randSeed", seed);

	window.draw(m_starLocalView, &m_starLocalViewShader);
}

void EffectsEmitter::drawPlanet(sf::RenderWindow& window, const sf::CircleShape& shape, float seed) {
	m_planetShader.setUniform("randSeed", seed);

	window.draw(shape, &m_planetShader);
}

void EffectsEmitter::drawGlow(sf::RenderWindow& window, const sf::Vector2f& pos, float size) {
	m_glow.setSize(sf::Vector2f(size, size));
	m_glow.setOrigin(sf::Vector2f(size / 2.0f, size / 2.0f));
	m_glow.setPosition(pos);

	m_glowShader.setUniform("size", sf::Glsl::Vec2(size, size));

	window.draw(m_glow, &m_glowShader);
}