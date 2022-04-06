#include "gamepch.h"

#include "EffectsEmitter.h"
#include "Math.h"
#include "Planet.h"
#include "Star.h"
#include "Renderer.h"
#include "GameState.h"
#include "Random.h"
#include "Camera.h"
#include "Fonts.h"

EffectsEmitter::EffectsEmitter(sf::Vector2i resolution, Renderer& renderer) :
m_renderer(renderer) {
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

	m_selectionShader.loadFromFile(m_vertexShaderPath, "data/shaders/selectionshader.shader");
	m_borderShader.loadFromFile(m_vertexShaderPath, "data/shaders/bordershader.shader");
	m_terraPlanetShader.loadFromFile(m_vertexShaderPath, "data/shaders/terraplanetshader.shader");
	m_blackHoleShader.loadFromFile(m_vertexShaderPath, "data/shaders/blackholeshader.shader");
	m_lavaPlanetShader.loadFromFile(m_vertexShaderPath, "data/shaders/lavaplanetshader.shader");

	m_mapStarShader.loadFromFile(m_vertexShaderPath, "data/shaders/fragmentshader3.shader");
	m_mapStarShader.setUniform("resolution", sf::Glsl::Vec2(resolution.x, resolution.y));

	m_ringsShader.loadFromFile(m_vertexShaderPath, "data/shaders/ringsshader.shader");
	m_asteroidBeltShader.loadFromFile("data/shaders/asteroidbeltvertex.shader", "data/shaders/newasteroidbeltshader.shader");
	m_postEffectsShader.loadFromFile(m_vertexShaderPath, "data/shaders/posteffectsshader.shader");
	m_parallaxShader.loadFromFile(m_vertexShaderPath, "data/shaders/parallaxshader.shader");
	m_lightningShader.loadFromFile(m_vertexShaderPath, "data/shaders/lightning.shader");
	m_jumpShader.loadFromFile(m_vertexShaderPath, "data/shaders/jumpshader.shader");
	m_jumpPointShader.loadFromFile(m_vertexShaderPath, "data/shaders/jumppointshader.shader");
	m_jumpTrailShader.loadFromFile(m_vertexShaderPath, "data/shaders/jumptrailshader.shader");
	m_particleShader.loadFromFile(m_vertexShaderPath, "data/shaders/particleshader.shader");
	m_beamShader.loadFromFile(m_vertexShaderPath, "data/shaders/beamshader.shader");
	m_planetMapShader.loadFromFile(m_vertexShaderPath, "data/shaders/planetmapshader.shader");
	m_terraPlanetMapShader.loadFromFile(m_vertexShaderPath, "data/shaders/terraplanetmapshader.shader");
	m_lavaPlanetMapShader.loadFromFile(m_vertexShaderPath, "data/shaders/lavaplanetmapshader.shader");
}

void EffectsEmitter::onEvent(const sf::Event& event) {
	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::F1) {
			initShaders(m_resolution);
			DEBUG_PRINT("Reloaded shaders");
		}
	}
}

void EffectsEmitter::drawJumpBubble(const sf::Vector2f& pos, float radius, float percent) {
	m_jumpBubble.setRadius(radius);
	m_jumpBubble.setOrigin(sf::Vector2f(radius, radius));
	m_jumpBubble.setFillColor(sf::Color(255.0f, 0.0f, 255.0f, 200.0f * (percent / 100.0f)));
	m_jumpBubble.setPosition(pos);
	m_renderer.draw(m_jumpBubble);
}

void EffectsEmitter::drawLine(const sf::Vector2f& begin, const sf::Vector2f& end, const sf::Color& color) {
	float angle = Math::angleBetween(begin, end);
	float dist = Math::distance(begin, end);

	m_line.setRotation(-angle);
	m_line.setSize(sf::Vector2f(dist, 5.0f));
	m_line.setFillColor(color);
	m_line.setPosition(begin);

	m_renderer.draw(m_line);
}

void EffectsEmitter::drawWithDistanceShader(sf::Shape& s, const sf::Vector2i& pos) {
	m_renderer.draw(s, &m_distanceShader);
}

void EffectsEmitter::drawFogOfWar() {
	sf::View oldView = m_renderer.getView();
	m_renderer.setView(m_renderer.getDefaultView());

	m_renderer.draw(m_fogOfWar);

	m_renderer.setView(oldView);
}

void EffectsEmitter::drawLocalStar(const sf::RectangleShape& starRect, float time, float seed) {
	m_starLocalViewShader.setUniform("time", time);
	m_starLocalViewShader.setUniform("randSeed", seed);

	m_renderer.draw(starRect, &m_starLocalViewShader);
}

void EffectsEmitter::drawPlanet(const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed, float time) {
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

	m_renderer.draw(shape, &m_planetShader);
}

void EffectsEmitter::drawTerraPlanet(const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed, float time) {
	m_terraPlanetShader.setUniform("size", sf::Glsl::Vec2(planet->getRadius(), planet->getRadius()));
	m_terraPlanetShader.setUniform("time", time);
	m_terraPlanetShader.setUniform("seed", seed);

	float angle = Math::angleBetween(planet->getPos(), star->getLocalViewCenter()) * Math::toRadians;
	m_terraPlanetShader.setUniform("sun", sf::Glsl::Vec2(std::cos(angle), -std::sin(angle)));

	m_renderer.draw(shape, &m_terraPlanetShader);
}

void EffectsEmitter::drawTerraPlanet(const sf::RectangleShape& shape, float planetRadius, sf::Vector2f planetPos, sf::Vector2f sunPos, float seed, float time) {
	m_terraPlanetShader.setUniform("size", sf::Glsl::Vec2(planetRadius, planetRadius));
	m_terraPlanetShader.setUniform("time", time);
	m_terraPlanetShader.setUniform("seed", seed);

	float angle = Math::angleBetween(planetPos, sunPos) * Math::toRadians;
	m_terraPlanetShader.setUniform("sun", sf::Glsl::Vec2(std::cos(angle), -std::sin(angle)));

	m_renderer.draw(shape, &m_terraPlanetShader);
}

void EffectsEmitter::drawLavaPlanet(const sf::RectangleShape& shape, const Planet* planet, const Star* star, float seed) {
	m_lavaPlanetShader.setUniform("size", sf::Glsl::Vec2(planet->getRadius(), planet->getRadius()));
	m_lavaPlanetShader.setUniform("seed", seed);

	float angle = Math::angleBetween(planet->getPos(), star->getLocalViewCenter()) * Math::toRadians;
	m_lavaPlanetShader.setUniform("sun", sf::Glsl::Vec2(std::cos(angle), -std::sin(angle)));

	m_renderer.draw(shape, &m_lavaPlanetShader);
}

void EffectsEmitter::drawGlow(const sf::Vector2f& pos, float size, const sf::Color& color) {
	m_glow.setSize(sf::Vector2f(size, size));
	m_glow.setOrigin(sf::Vector2f(size / 2.0f, size / 2.0f));
	m_glow.setPosition(pos);
	m_glow.setFillColor(color);

	m_glowShader.setUniform("size", sf::Glsl::Vec2(size, size));

	m_renderer.draw(m_glow, &m_glowShader);
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

void EffectsEmitter::drawNebula(sf::Sprite& sprite, float seed) {
	m_nebulaShader.setUniform("background", sf::Shader::CurrentTexture);
	m_nebulaShader.setUniform("seed", seed);
	m_renderer.draw(sprite, &m_nebulaShader);
}

void EffectsEmitter::drawSelection(sf::Vector2f pos, float radius) {
	sf::RectangleShape selectionIndicator;

	selectionIndicator.setOrigin(sf::Vector2f(radius * 2.0f, radius * 2.0f));
	selectionIndicator.setPosition(pos);
	selectionIndicator.setFillColor(sf::Color::Yellow);
	selectionIndicator.setSize(sf::Vector2f(radius * 4.0f, radius * 4.0f));
	selectionIndicator.setTextureRect(sf::IntRect({ 0, 0 }, { 1, 1 }));
	
	m_renderer.draw(selectionIndicator, &m_selectionShader);
}

void EffectsEmitter::updateTime(float time, float gameTime) {
	m_lastTime = time;
	m_lastGameTime = gameTime;
	m_selectionShader.setUniform("time", time);
	m_mapStarShader.setUniform("time", time);
	m_ringsShader.setUniform("time", time);
	m_asteroidBeltShader.setUniform("time", time);
	m_postEffectsShader.setUniform("time", gameTime);
}

void EffectsEmitter::drawBorders(const sf::RectangleShape& shape, const std::vector<sf::Glsl::Vec3>& points, sf::Color color) {
	m_borderShader.setUniformArray("points", points.data(), points.size());
	m_borderShader.setUniform("numPoints", static_cast<int>(points.size()));
	m_borderShader.setUniform("size", sf::Glsl::Vec2(shape.getSize().x, shape.getSize().y));
	m_borderShader.setUniform("color", sf::Glsl::Vec3(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f));
	m_renderer.draw(shape, &m_borderShader);
}

void EffectsEmitter::drawBlackHole(const sf::RectangleShape& starRect, float time, float seed) {
	m_blackHoleShader.setUniform("time", time);
	m_blackHoleShader.setUniform("randSeed", seed);
	m_blackHoleShader.setUniform("size", starRect.getSize());

	m_renderer.draw(starRect, &m_blackHoleShader);
}

void EffectsEmitter::drawMapStar(const sf::CircleShape& shape, bool flashing) {
	m_mapStarShader.setUniform("radius", shape.getRadius());
	m_mapStarShader.setUniform("flashing", flashing);
	m_renderer.draw(shape, &m_mapStarShader);
}

void EffectsEmitter::drawRings(sf::Vector2f pos, float radius, float seed) {
	m_ringsShader.setUniform("seed", seed);
	m_ringsShader.setUniform("size", sf::Glsl::Vec2(radius, radius));

	sf::RectangleShape shape;
	shape.setSize(sf::Vector2f(radius, radius));
	shape.setOrigin(sf::Vector2f(radius / 2.0f, radius / 2.0f));
	shape.setPosition(pos);

	m_renderer.draw(shape, &m_ringsShader);
}

void EffectsEmitter::drawAsteroidBelt(const std::vector<sf::Vertex>& vertices, float rotationSpeed, sf::Vector2f sunPos) {
	m_asteroidBeltShader.setUniform("tex", TextureCache::getTexture("data/art/asteroidnormalssheet.png"));
	m_asteroidBeltShader.setUniform("rotationSpeed", rotationSpeed);
	m_asteroidBeltShader.setUniform("sunPos", sunPos);

	m_renderer.draw(&vertices[0], vertices.size(), sf::Quads, &m_asteroidBeltShader);
}

void EffectsEmitter::drawPostEffects(sf::Sprite& sprite, sf::RenderWindow& window, GameState& state) {
	Star* star = state.getLocalViewStar();
	bool blackHole = false;

	if (star != nullptr) {
		blackHole = star->isBlackHole();
	}

	// Black hole post processing effect
	sf::Vector2f texSize = sf::Vector2f(sprite.getTexture()->getSize());

	m_postEffectsShader.setUniform("screen", *sprite.getTexture());
	m_postEffectsShader.setUniform("aspect", texSize.x / texSize.y);
	m_postEffectsShader.setUniform("zoom", state.getCamera().getZoomFactor());

	// Black hole effect
	if (blackHole) {
		sf::Vector2f bhPos = star->getLocalViewCenter();
		m_postEffectsShader.setUniform("blackHole", true);
		m_postEffectsShader.setUniform("blackHolePos", m_renderer.worldToScreenPos(bhPos));
	}
	else {
		m_postEffectsShader.setUniform("blackHole", false);
	}

	// Explosion effect
	if (m_explosionEffects.size() > 0) {
		if (state.getState() == GameState::State::LOCAL_VIEW) {
			std::vector<sf::Glsl::Vec2> explosionPoints;
			std::vector<float> explosionTimes;

			for (ExplosionEffect& explosion : m_explosionEffects) {
				if (explosion.star == star) {
					explosionPoints.push_back(m_renderer.worldToScreenPos(explosion.point));
					explosionTimes.push_back(explosion.time);
				}
			}

			m_postEffectsShader.setUniform("numExplosions", (int)explosionPoints.size());

			if (explosionPoints.size() > 0) {
				m_postEffectsShader.setUniformArray("explosionPoints", explosionPoints.data(), explosionPoints.size());
				m_postEffectsShader.setUniformArray("explosionTimes", explosionTimes.data(), explosionTimes.size());
			}
		}
		else {
			m_postEffectsShader.setUniform("numExplosions", 0);
		}

		if (m_lastGameTime - m_explosionEffects[0].time > 3.0f) {
			m_explosionEffects.erase(m_explosionEffects.begin());
		}
	}
	else {
		m_postEffectsShader.setUniform("numExplosions", 0);
	}

	window.draw(sprite, &m_postEffectsShader);
}

void EffectsEmitter::drawLaserAnimation(sf::Vector2f sourcePos, sf::Vector2f endPos, float step) {
	float angle = Math::angleBetween(sourcePos, endPos);
	sf::RectangleShape shape;

	shape.setRotation(-angle);
	shape.setPosition(sourcePos);
	shape.setSize(sf::Vector2f(Math::distance(sourcePos, endPos), 25.0f));
	shape.setFillColor(sf::Color(255, 0, 0, 255 * (1.0 / step)));

	m_renderer.draw(shape);
}

void EffectsEmitter::drawGatlingAnimation(sf::Vector2f sourcePos, sf::Vector2f endPos, float step) {
	float angle = Math::angleBetween(sourcePos, endPos);
	float dist = Math::distance(sourcePos, endPos);
	float offset = 10000.0f * std::fmod(step * 10.0f, 1.0f);

	if (offset < dist && step < 0.5f) {
		sf::Vector2f pos(sourcePos.x + std::cos(angle * Math::toRadians) * offset, sourcePos.y + -std::sin(angle * Math::toRadians) * offset);

		sf::Sprite sprite;

		sprite.setTexture(TextureCache::getTexture("data/art/bullet2.png"));
		sprite.setRotation(-angle + 90.0f);
		sprite.setPosition(pos);

		m_renderer.draw(sprite);
	}
}

void EffectsEmitter::drawGatlingLaserAnimation(sf::Vector2f sourcePos, sf::Vector2f endPos, float step) {
	float angle = Math::angleBetween(sourcePos, endPos);
	float dist = Math::distance(sourcePos, endPos);
	float offset = 10000.0f * std::fmod(step * 10.0f, 1.0f);

	if (offset < dist && step < 0.5f) {
		sf::Vector2f pos(sourcePos.x + std::cos(angle * Math::toRadians) * offset, sourcePos.y + -std::sin(angle * Math::toRadians) * offset);

		sf::Sprite sprite;

		sprite.setTexture(TextureCache::getTexture("data/art/laser.png"));
		sprite.setScale({ 2.0f, 2.0f });
		sprite.setRotation(-angle + 90.0f);
		sprite.setOrigin({ sprite.getTextureRect().width / 2.0f, sprite.getTextureRect().height / 2.0f });
		sprite.setPosition(pos);

		m_renderer.draw(sprite);
	}
}

void EffectsEmitter::drawBeamAnimation(sf::Vector2f sourcePos, sf::Vector2f endPos, float step, sf::Color color) {
	float angle = Math::angleBetween(sourcePos, endPos);
	sf::RectangleShape shape;

	shape.setRotation(-angle);
	shape.setPosition(sourcePos);
	shape.setSize(sf::Vector2f(Math::distance(sourcePos, endPos), 250.0f));
	shape.setOrigin({ 0.0f, shape.getSize().y / 2.0f });
	shape.setFillColor(sf::Color(color.r, color.g, color.b, color.a * (1.0 / step)));
	shape.setTextureRect(sf::IntRect({ 0, 0 }, { 1, 1 }));

	m_beamShader.setUniform("time", step);

	m_renderer.draw(shape, &m_beamShader);
}

void EffectsEmitter::addExplosionEffect(sf::Vector2f pos, Star* star) {
	m_explosionEffects.push_back(ExplosionEffect{
		pos,
		m_lastGameTime,
		star
	});
}

void EffectsEmitter::drawParallaxBackground(Camera& camera) {
	sf::RectangleShape shape;
	shape.setSize(sf::Vector2f(m_resolution));
	m_parallaxShader.setUniform("size", sf::Glsl::Vec2(m_resolution));
	m_parallaxShader.setUniform("cameraPos", sf::Glsl::Vec2(camera.getPos()));
	m_parallaxShader.setUniform("zoom", camera.getZoomFactor());

	m_renderer.draw(shape, &m_parallaxShader);
}

void EffectsEmitter::drawLightningEffect(sf::Vector2f pos, float time, float seed) {
	sf::RectangleShape shape;
	shape.setPosition(pos);
	shape.setSize({ 5000.0f, 5000.0f });
	shape.setOrigin({ 2500.0f, 2500.0f });

	m_lightningShader.setUniform("size", sf::Glsl::Vec2(5000.0f, 5000.0f));
	m_lightningShader.setUniform("time", time);
	m_lightningShader.setUniform("seed", seed);

	m_renderer.draw(shape, &m_lightningShader);
}

void EffectsEmitter::drawJumpEffect(sf::Vector2f pos, float time, float seed) {
	sf::RectangleShape shape;
	shape.setPosition(pos);
	shape.setSize({ 1000.0f, 1000.0f });
	shape.setOrigin({ 500.0f, 500.0f });

	m_jumpShader.setUniform("size", sf::Glsl::Vec2(1000.0f, 1000.0f));
	m_jumpShader.setUniform("time", time);
	m_jumpShader.setUniform("seed", seed);

	m_renderer.draw(shape, &m_jumpShader);
}

void EffectsEmitter::drawJumpPoint(sf::Vector2f pos, float time, float seed) {
	sf::RectangleShape shape;
	shape.setPosition(pos);
	shape.setSize({ 1250.0f, 1250.0f });
	shape.setOrigin({ 625.0f, 625.0f });

	m_jumpPointShader.setUniform("size", sf::Glsl::Vec2(1250.0f, 1250.0f));
	m_jumpPointShader.setUniform("time", time);
	m_jumpPointShader.setUniform("seed", seed);

	m_renderer.draw(shape, &m_jumpPointShader);
}

void EffectsEmitter::drawJumpTrail(const sf::RectangleShape& shape) {
	m_jumpTrailShader.setUniform("size", sf::Glsl::Vec2(shape.getSize().x / 2.0f, shape.getSize().y / 2.0f));
	m_renderer.draw(shape, &m_jumpTrailShader);
}

void EffectsEmitter::drawParticles(const std::vector<sf::Vertex>& vertices) {
	sf::RenderStates states;
	states.blendMode = sf::BlendAdd;
	states.shader = &m_particleShader;
	m_renderer.draw(&vertices[0], vertices.size(), sf::Quads, states);
}

void EffectsEmitter::drawPlanetMap(tgui::Canvas* canvas, Planet& planet, const sf::RenderWindow& window, bool showPopulation, sf::Vector2i selectedTile) {
	sf::RectangleShape shape;
	shape.setSize(canvas->getSize());
	shape.setFillColor(planet.getColor());
	shape.setTextureRect({ 0, 0, 1, 1 });

	if (planet.getType() == Planet::PLANET_TYPE::TERRA) {
		m_terraPlanetMapShader.setUniform("seed", planet.getShaderSeed());

		canvas->draw(shape, &m_terraPlanetMapShader);
	}
	else if (planet.getType() == Planet::PLANET_TYPE::LAVA || planet.getType() == Planet::PLANET_TYPE::VOLCANIC) {
		m_lavaPlanetMapShader.setUniform("seed", planet.getShaderSeed());

		canvas->draw(shape, &m_lavaPlanetMapShader);
	}
	else {
		m_planetMapShader.setUniform("size", sf::Glsl::Vec2(canvas->getSize().x, canvas->getSize().y));
		m_planetMapShader.setUniform("randSeed", planet.getShaderSeed());
		m_planetMapShader.setUniform("gasGiant", planet.isGasGiant());
		m_planetMapShader.setUniform("frozen", planet.getTemperature() < 273.15f);
		m_planetMapShader.setUniform("water", planet.getWater());

		canvas->draw(shape, &m_planetMapShader);
	}

	const int gridSize = 8;
	const sf::Vector2f gridRectSize = canvas->getSize() / gridSize;

	// Draw grid
	sf::RectangleShape gridRect;
	gridRect.setSize(gridRectSize);
	gridRect.setFillColor(sf::Color::Transparent);
	gridRect.setOutlineThickness(1.0f);
	gridRect.setOutlineColor(sf::Color(55, 55, 55));

	sf::Text text;
	text.setFont(Fonts::getMainFont());
	text.setColor(sf::Color(55, 55, 55));

	const Colony& colony = planet.getColony();

	for (int y = 0; y < gridSize; y++) {
		for (int x = 0; x < gridSize; x++) {
			sf::Vector2f pos(x * gridRectSize.x, y * gridRectSize.y);

			gridRect.setPosition(pos);

			sf::FloatRect relBounds = gridRect.getGlobalBounds();
			relBounds.left += canvas->getAbsolutePosition().x;
			relBounds.top += canvas->getAbsolutePosition().y;

			if (relBounds.contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
				gridRect.setFillColor(sf::Color(255, 255, 255, 137));
				gridRect.setTexture(nullptr);
			}
			else {
				if (colony.isGridGenerated()) {
					const Colony::Tile& tile = colony.getTile({ x, y });
					
					if (tile.hidden) {
						text.setString("?");
						text.setPosition(pos);
						text.setColor(sf::Color::Red);

						gridRect.setTexture(nullptr);
						gridRect.setFillColor(sf::Color::Transparent);
					}
					else if (tile.population > 0) {
						if (showPopulation) {
							text.setString(std::to_string(tile.population));
							text.setPosition(pos);
							text.setColor(sf::Color(55, 55, 55));
						}

						gridRect.setFillColor(sf::Color::White);
						gridRect.setTexture(&TextureCache::getTexture(Colony::getCityTexturePath(tile.population, tile.cityVariant)), true);
					}
					else {
						gridRect.setFillColor(sf::Color::Transparent);
						gridRect.setTexture(nullptr);
					}
				}
				else {
					gridRect.setFillColor(sf::Color::Transparent);
				}
			}

			if (sf::Vector2i{ x, y } == selectedTile) {
				gridRect.setOutlineColor(sf::Color::Yellow);
			}
			else {
				gridRect.setOutlineColor(sf::Color(55, 55, 55));
			}
			
			canvas->draw(gridRect);

			if (text.getString() != "") {
				canvas->draw(text);
				text.setString("");
			}
		}
	}
}