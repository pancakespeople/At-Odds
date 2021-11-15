#include "gamepch.h"

#include "EffectsEmitter.h"
#include "Math.h"
#include "Planet.h"
#include "Star.h"

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
	m_distanceShader.loadFromFile(m_vertexShaderPath, "data/shaders/distfragshader.shader");
	m_distanceShader.setUniform("resolution", sf::Glsl::Vec2(resolution.x, resolution.y));
	
	m_starLocalViewShader.loadFromFile(m_vertexShaderPath, "data/shaders/starlocalview.shader");
	m_starLocalViewShader.setUniform("size", sf::Glsl::Vec2(2048, 2048));

	m_planetShader.loadFromFile(m_vertexShaderPath, "data/shaders/planetshader.shader");
	m_planetShader.setUniform("size", sf::Glsl::Vec2(2048, 2048));

	m_glowShader.loadFromFile(m_vertexShaderPath, "data/shaders/glow.shader");

	m_nebulaShader.loadFromFile(m_vertexShaderPath, "data/shaders/nebulashader.shader");
	m_nebulaShader.setUniform("size", sf::Glsl::Vec2(resolution.x, resolution.y));

	m_selectionShader.loadFromFile(m_vertexShaderPath, "data/shaders/selectionshader.shader");
	m_borderShader.loadFromFile(m_vertexShaderPath, "data/shaders/bordershader.shader");
	m_terraPlanetShader.loadFromFile(m_vertexShaderPath, "data/shaders/terraplanetshader.shader");
	m_blackHoleShader.loadFromFile(m_vertexShaderPath, "data/shaders/blackholeshader.shader");
	m_lavaPlanetShader.loadFromFile(m_vertexShaderPath, "data/shaders/lavaplanetshader.shader");

	m_mapStarShader.loadFromFile(m_vertexShaderPath, "data/shaders/fragmentshader3.shader");
	m_mapStarShader.setUniform("resolution", sf::Glsl::Vec2(resolution.x, resolution.y));

	m_ringsShader.loadFromFile(m_vertexShaderPath, "data/shaders/ringsshader.shader");
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

void EffectsEmitter::drawLocalStar(sf::RenderWindow& window, const sf::RectangleShape& starRect, float time, float seed) {
	m_starLocalViewShader.setUniform("time", time);
	m_starLocalViewShader.setUniform("randSeed", seed);

	window.draw(starRect, &m_starLocalViewShader);
}

void EffectsEmitter::drawPlanet(sf::RenderWindow& window, const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed, float time) {
	m_planetShader.setUniform("randSeed", seed);
	m_planetShader.setUniform("size", sf::Glsl::Vec2(planet->getRadius(), planet->getRadius()));
	m_planetShader.setUniform("time", time);
	m_planetShader.setUniform("gasGiant", planet->isGasGiant());
	m_planetShader.setUniform("water", planet->getWater());
	m_planetShader.setUniform("frozen", planet->getTemperature() < 273.15f);
	m_planetShader.setUniform("atmosphere", planet->getAtmosphericPressure() > 0.1f && 
		planet->getType() != Planet::PLANET_TYPE::GAS_GIANT && planet->getType() != Planet::PLANET_TYPE::ICE_GIANT);

	switch (planet->getType()) {
	case Planet::PLANET_TYPE::GAS_GIANT:
	case Planet::PLANET_TYPE::ICE_GIANT:
	case Planet::PLANET_TYPE::TOXIC:
		m_planetShader.setUniform("atmosSameColor", true);
		break;
	default:
		m_planetShader.setUniform("atmosSameColor", false);
	}

	float angle = Math::angleBetween(planet->getPos(), star->getLocalViewCenter()) * Math::toRadians;
	m_planetShader.setUniform("sunVec", sf::Glsl::Vec2(std::cos(angle), -std::sin(angle)));

	window.draw(shape, &m_planetShader);
}

void EffectsEmitter::drawTerraPlanet(sf::RenderWindow& window, const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed, float time) {
	m_terraPlanetShader.setUniform("size", sf::Glsl::Vec2(planet->getRadius(), planet->getRadius()));
	m_terraPlanetShader.setUniform("time", time);
	m_terraPlanetShader.setUniform("seed", seed);

	float angle = Math::angleBetween(planet->getPos(), star->getLocalViewCenter()) * Math::toRadians;
	m_terraPlanetShader.setUniform("sun", sf::Glsl::Vec2(std::cos(angle), -std::sin(angle)));

	window.draw(shape, &m_terraPlanetShader);
}

void EffectsEmitter::drawLavaPlanet(sf::RenderWindow& window, const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed) {
	m_lavaPlanetShader.setUniform("size", sf::Glsl::Vec2(planet->getRadius(), planet->getRadius()));
	m_lavaPlanetShader.setUniform("seed", seed);

	float angle = Math::angleBetween(planet->getPos(), star->getLocalViewCenter()) * Math::toRadians;
	m_lavaPlanetShader.setUniform("sun", sf::Glsl::Vec2(std::cos(angle), -std::sin(angle)));

	window.draw(shape, &m_lavaPlanetShader);
}

void EffectsEmitter::drawGlow(sf::RenderWindow& window, const sf::Vector2f& pos, float size, const sf::Color& color) {
	m_glow.setSize(sf::Vector2f(size, size));
	m_glow.setOrigin(sf::Vector2f(size / 2.0f, size / 2.0f));
	m_glow.setPosition(pos);
	m_glow.setFillColor(color);

	m_glowShader.setUniform("size", sf::Glsl::Vec2(size, size));

	window.draw(m_glow, &m_glowShader);
}

void EffectsEmitter::drawHabitableZone(sf::RenderWindow& window, const sf::Vector2f& starPos, float temperature) {
	float thinnestAtmosTemp = temperature + std::max(0.0f, std::log(0.5f + 0.25f) * temperature);
	float thickestAtmosTemp = temperature + std::max(0.0f, std::log(3.0f + 0.25f) * temperature);
	
	float habitableInnerRadius = (thickestAtmosTemp * 1000.0f) / 325.0f;
	float habitableOuterRadius = (thinnestAtmosTemp * 1000.0f) / 273.15f;

	m_habitableZone.setPosition(starPos);
	m_habitableZone.setRadius(habitableInnerRadius);
	m_habitableZone.setOrigin(sf::Vector2f(habitableInnerRadius, habitableInnerRadius));
	m_habitableZone.setFillColor(sf::Color::Transparent);
	m_habitableZone.setOutlineColor(sf::Color::Green);
	m_habitableZone.setOutlineThickness(25.0f);

	window.draw(m_habitableZone);
	m_habitableZone.setRadius(habitableOuterRadius);
	m_habitableZone.setOrigin(sf::Vector2f(habitableOuterRadius, habitableOuterRadius));
	window.draw(m_habitableZone);
}

void EffectsEmitter::drawNebula(sf::RenderWindow& window, sf::Sprite& sprite, float seed) {
	m_nebulaShader.setUniform("background", sf::Shader::CurrentTexture);
	m_nebulaShader.setUniform("seed", seed);
	window.draw(sprite, &m_nebulaShader);
}

void EffectsEmitter::drawSelection(sf::RenderWindow& window, const sf::RectangleShape& shape) {
	m_selectionShader.setUniform("size", sf::Glsl::Vec2(shape.getSize()));
	window.draw(shape, &m_selectionShader);
}

void EffectsEmitter::updateTime(float time) {
	m_selectionShader.setUniform("time", time);
	m_mapStarShader.setUniform("time", time);
	m_ringsShader.setUniform("time", time);
}

void EffectsEmitter::drawBorders(sf::RenderWindow& window, const sf::RectangleShape& shape, const std::vector<sf::Glsl::Vec2>& points, sf::Color color) {
	m_borderShader.setUniformArray("points", points.data(), points.size());
	m_borderShader.setUniform("numPoints", static_cast<int>(points.size()));
	m_borderShader.setUniform("size", sf::Glsl::Vec2(shape.getSize().x, shape.getSize().y));
	m_borderShader.setUniform("color", sf::Glsl::Vec3(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f));
	window.draw(shape, &m_borderShader);
}

void EffectsEmitter::drawBlackHole(sf::RenderWindow& window, const sf::RectangleShape& starRect, float time, float seed) {
	m_blackHoleShader.setUniform("time", time);
	m_blackHoleShader.setUniform("randSeed", seed);
	m_blackHoleShader.setUniform("size", starRect.getSize());

	window.draw(starRect, &m_blackHoleShader);
}

void EffectsEmitter::drawMapStar(sf::RenderWindow& window, const sf::CircleShape& shape, bool flashing) {
	m_mapStarShader.setUniform("radius", shape.getRadius());
	m_mapStarShader.setUniform("flashing", flashing);
	window.draw(shape, &m_mapStarShader);
}

void EffectsEmitter::drawRings(sf::RenderWindow& window, sf::Vector2f pos, float radius, float seed) {
	m_ringsShader.setUniform("seed", seed);
	m_ringsShader.setUniform("size", sf::Glsl::Vec2(radius, radius));

	sf::RectangleShape shape;
	shape.setSize(sf::Vector2f(radius, radius));
	shape.setOrigin(sf::Vector2f(radius / 2.0f, radius / 2.0f));
	shape.setPosition(pos);

	window.draw(shape, &m_ringsShader);
}