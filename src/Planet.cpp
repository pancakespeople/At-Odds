#include "gamepch.h"
#include "Planet.h"
#include "Random.h"
#include "Math.h"
#include "Mod.h"
#include "Star.h"
#include "Faction.h"
#include "Projectile.h"
#include "Building.h"
#include "Util.h"
#include "SmoothCircle.h"
#include "Renderer.h"
#include "Fonts.h"

Planet::Planet(sf::Vector2f pos, sf::Vector2f starPos, sf::Vector2f orbitPos, float starTemperature, bool moon) {
	m_shape.setFillColor(sf::Color(155, 155, 155));
	m_shape.setSize(sf::Vector2f(500.0f, 500.0f));
	m_shape.setPosition(pos);
	m_shape.setTextureRect({ 0, 0, 1, 1 });

	float radiusFromStar = Math::distance(pos, starPos);
	float baseTemperature = std::min(starTemperature, (starTemperature * 1000.0f) / radiusFromStar);

	m_orbit = Orbit(pos, orbitPos);

	if (baseTemperature < 273.15f) {
		if (Random::randFloat(0.0f, 1.0f) < 0.5f && !moon) {
			// Gas giant

			generateGasGiant(baseTemperature);
		}
		else {
			// Rocky planet

			generateTerrestrial(baseTemperature);
		}
	}
	else {
		if (Random::randFloat(0.0f, 1.0f) < 0.9f || moon) {
			// Rocky planet

			generateTerrestrial(baseTemperature);
		}
		else {
			// Gas giant

			generateGasGiant(baseTemperature);
		}
	}

	//generateResources();
	m_shaderRandomSeed = Random::randFloat(0.0f, 1.0f);

	m_shape.setOrigin(sf::Vector2f(getRadius(), getRadius()));
}

void Planet::draw(Renderer& renderer, sf::RenderWindow& window, Star* star, float time, bool drawPlanetText) {
	//m_orbit.draw(window);
	
	if (isMouseInRadius(window, renderer) && drawPlanetText) {
		renderer.effects.drawGlow(m_shape.getPosition(), getRadius() * 5.0f, m_shape.getFillColor());

		// Hover text
		sf::Text text;
		text.setFont(Fonts::getFont(Fonts::MAIN_FONT_PATH));
		text.setString(getName(star) + " - Population: " + std::to_string(getColony().getPopulation()));
		text.setPosition(getPos() - sf::Vector2f{0.0f, getRadius()*2.0f});
		text.setScale({ getRadius() / 100.0f, getRadius() / 100.0f });
		text.setOrigin({ text.getLocalBounds().width / 2.0f, 0.0f });

		renderer.draw(text);
	}

	switch (m_type) {
	case PLANET_TYPE::TERRA:
		renderer.effects.drawTerraPlanet(m_shape, this, star, m_shaderRandomSeed, time);
		break;
	case PLANET_TYPE::LAVA:
	case PLANET_TYPE::VOLCANIC:
		renderer.effects.drawLavaPlanet(m_shape, this, star, m_shaderRandomSeed);
		break;
	default:
		renderer.effects.drawPlanet(m_shape, this, star, m_shaderRandomSeed, time);
	}

	if (m_colony.getAllegiance() != -1) {
		// Draw faction indicator circle

		SmoothCircle circle;
		circle.setRadius(getRadius() + 100.0f);
		circle.setColor(m_colony.getFactionColor());
		circle.setOutlineThickness(25.0f);
		circle.setOrigin(sf::Vector2f(circle.getRadius(), circle.getRadius()));
		circle.setPosition(m_shape.getPosition());

		renderer.draw(circle);
	}

	for (Projectile& proj : m_bombardProjectiles) {
		proj.draw(renderer);
	}

	if (m_rings) renderer.effects.drawRings(getPos(), getRadius() * 8.0f, m_shaderRandomSeed);
}

void Planet::update(Star* currentStar, Faction* faction) {
	m_shape.setPosition(m_orbit.update());
	m_colony.update(currentStar, faction, this);

	// Bombard projectiles
	for (int i = 0; i < m_bombardProjectiles.size(); i++) {
		m_bombardProjectiles[i].setRotation(Math::angleBetween(m_bombardProjectiles[i].getPos(), getPos()));
		m_bombardProjectiles[i].update(currentStar);

		float dist = Math::distance(m_bombardProjectiles[i].getPos(), getPos());
		if (dist < getRadius()) {
			m_bombardProjectiles[i].onDeath(currentStar);

			if (m_bombardProjectiles[i].canOrbitallyBombard()) {

				float deathPercent = Random::randFloat(0.0f, 0.05f);
				float deathConstant = m_bombardProjectiles[i].getDamage();

				m_colony.changeWorldPopulation(-(m_colony.getPopulation() * deathPercent + deathConstant) * m_colony.getBuildingEffects("bombardDamageMultiplier"));

			}

			if (m_bombardProjectiles[i].canInvadePlanets()) {
				float deathConstant = Random::randInt(100, 1000);
				float invasionEffectiveness = m_colony.getBuildingEffects("invasionEffectiveness");

				m_colony.changeWorldPopulation(-deathConstant * invasionEffectiveness);

				if (m_colony.getPopulation() == 0) {
					DEBUG_PRINT("Set allegiance to " << m_bombardProjectiles[i].getAllegiance());
					m_colony.setAllegiance(m_bombardProjectiles[i].getAllegiance());
					m_colony.onColonization(*this);
					m_colony.changePopulation(Random::randInt(100, 1000), m_colony.getRandomTile());
					m_colony.setFactionColonyLegality(m_bombardProjectiles[i].getAllegiance(), true);
				}
			}

			m_bombardProjectiles.erase(m_bombardProjectiles.begin() + i);
			i--;
		}
	}

	m_timeSinceCreaton += 1.0f / 60.0f; // Fixed timestep, 60 updates per second

	// Set orbit pos
	if (isMoon()) {
		Planet& parent = currentStar->getPlanets()[m_parentPlanetIndex];
		m_orbit.setParentPos(parent.getPos());
	}
}

void Planet::generateGasGiant(float baseTemperature) {
	float radius = getRadius() * Random::randFloat(2.0f, 4.0f);
	m_shape.setSize(sf::Vector2f(radius * 2.0f, radius * 2.0f));
	
	if (baseTemperature < 100.0f) {
		// Blue
		int otherColors = Random::randInt(0, 155);
		m_shape.setFillColor(sf::Color(otherColors, otherColors, Random::randInt(155, 255)));

		m_type = PLANET_TYPE::ICE_GIANT;
	}
	else {
		// Orange
		int r = Random::randInt(155, 255);
		int g = r / 1.5f;
		int b = Random::randInt(0, 155);

		m_shape.setFillColor(sf::Color(r, g, b));

		m_type = PLANET_TYPE::GAS_GIANT;
	}

	m_atmosphere = Random::randFloat(1000.0f, 1000000.0f);
	m_temperature = baseTemperature;

	m_gasGiant = true;
	if (Random::randFloat(0.0f, 1.0f) < 0.33f) m_rings = true;
}

void Planet::generateTerrestrial(float baseTemperature) {
	float radius = getRadius() * Random::randFloat(0.75f, 1.9f);
	m_shape.setSize(sf::Vector2f(radius * 2.0f, radius * 2.0f));

	if (baseTemperature > 400.0f) {
		// Too hot to sustain an atmosphere

		m_atmosphere = 0.0f;
	}
	else {
		float rnd = Random::randFloat(0.0, 1.0);
		if (rnd > 0.9) {
			// 10% chance to be a toxic world

			m_atmosphere = Random::randFloat(50.0f, 200.0f);
		}
		else if (rnd > 0.45) {
			// Earth like atmosphere

			m_atmosphere = Random::randFloat(0.0f, 3.0f);
		}
		else {
			// No atmosphere

			m_atmosphere = 0.0f;
		}
	}

	m_temperature = baseTemperature + std::max(0.0f, std::log(m_atmosphere + 0.25f) * baseTemperature);

	if (m_temperature > 325.0f || m_atmosphere == 0.0f) {
		m_water = 0.0f;
	}
	else {
		m_water = Random::randFloat(0.0f, 1.0f);
	}

	// Types
	if (m_temperature > 273.15 && m_temperature < 325.0f &&
		m_atmosphere > 0.5f && m_atmosphere < 3.0f &&
		m_water > 0.4f) {
		// Terra

		m_shape.setFillColor(sf::Color::Green);
		m_type = PLANET_TYPE::TERRA;
	}
	else {
		if (m_atmosphere >= 50.0f) {
			// Toxic

			m_shape.setFillColor(sf::Color::Yellow);
			m_type = PLANET_TYPE::TOXIC;
		}
		else if (m_water == 0.0f && m_atmosphere == 0.0f) {
			if (m_temperature > 2000.0f) {
				// Lava
				// Red

				int gb = std::min(255.0f, m_temperature * 0.05f);

				m_shape.setFillColor(sf::Color(255, 100, 100));
				m_type = PLANET_TYPE::LAVA;
			}
			else {
				int rnd = Random::randInt(0, 3);

				if (rnd < 3) {
					// Mercury
					// Gray/white

					int rgb = Random::randInt(125, 255);

					m_shape.setFillColor(sf::Color(rgb, rgb, rgb));
					m_type = PLANET_TYPE::BARREN;
				}
				else {
					// Volcanic

					int gb = std::min(255.0f, m_temperature * 0.05f);

					m_shape.setFillColor(sf::Color(255, 100, 100));
					m_type = PLANET_TYPE::VOLCANIC;
					m_temperature += Random::randFloat(50.0f, 200.0f);
				}
			}
		}
		else if (m_water > 0.4f && m_temperature < 273.15f) {
			// Tundra
			// White

			m_shape.setFillColor(sf::Color::White);
			m_type = PLANET_TYPE::TUNDRA;
		}
		else if (m_water > 0.6f) {
			// Ocean

			int rgb = Random::randInt(50, 150);
			m_shape.setFillColor(sf::Color(rgb, rgb, rgb));
			m_type = PLANET_TYPE::OCEAN;
		}
		else {
			// Desert/Martian

			if (Random::randBool()) {
				// Orange

				int r = Random::randInt(155, 255);
				int g = r / 1.5f;
				int b = Random::randInt(0, 155);

				m_shape.setFillColor(sf::Color(r, g, b));
			}
			else {
				// Sand

				int r = Random::randInt(155, 255);
				int g = std::max(0, r - 36);
				int b = std::max(0, r - 62);

				m_shape.setFillColor(sf::Color(r, g, b));
				
			}

			m_type = PLANET_TYPE::DESERT;
		}
	}

	if (Random::randFloat(0.0f, 1.0f) < 0.1f) m_rings = true;
}

std::string Planet::getTypeString() const {
	switch (m_type) {
	case PLANET_TYPE::BARREN:
		return "Barren";
	case PLANET_TYPE::DESERT:
		return "Desert";
	case PLANET_TYPE::GAS_GIANT:
		return "Gas Giant";
	case PLANET_TYPE::ICE_GIANT:
		return "Ice Giant";
	case PLANET_TYPE::TERRA:
		return "Terra";
	case PLANET_TYPE::TOXIC:
		return "Toxic";
	case PLANET_TYPE::TUNDRA:
		return "Tundra";
	case PLANET_TYPE::LAVA:
		return "Lava";
	case PLANET_TYPE::OCEAN:
		return "Ocean";
	case PLANET_TYPE::VOLCANIC:
		return "Volcanic";
	default:
		return "Unknown";
	}
}

float Planet::getHabitability() const {
	const float temperatureGoal = 288.0f;
	const float atmosGoal = 1.0f;
	const float waterGoal = 0.71f;

	float temperatureDiff = std::abs(m_temperature - temperatureGoal) / 288.0f;
	float atmosDiff = std::abs(m_atmosphere - atmosGoal);
	float waterDiff = std::abs(m_water - waterGoal);

	float diff = temperatureDiff + atmosDiff + waterDiff;
	if (diff == 0.0f) {
		return 1000000;
	}
	else {
		return 1.0f / diff;
	}
}

void Planet::onColonization() {
	m_colony.setTicksToNextBus(HabitatMod::calcBusTickTimer(m_colony.getPopulation()));
	//m_colony.addBuilding(ColonyBuilding("INFRASTRUCTURE"));

	m_colony.getTradeGoods().addSupply("WATER", 100.0f);
	m_colony.getTradeGoods().addSupply("FOOD", 100.0f);

	m_colony.onColonization(*this);

	addEvent("COLONY_START");
}

void Planet::createSpaceBus(sf::Color factionColor, Star* currentStar, Star* targetStar, Planet* targetPlanet, const std::string& type, int population, int maxPopulation) {
	Spaceship* bus = currentStar->createSpaceship(type, getPos(), m_colony.getAllegiance(), factionColor);
	bus->addMod(HabitatMod(population, maxPopulation, false));
	bus->addOrder(TravelOrder(targetStar));
	bus->addOrder(InteractWithPlanetOrder(targetPlanet, targetStar));
	bus->addOrder(TravelOrder(currentStar));
	bus->addOrder(InteractWithPlanetOrder(this, currentStar));
	bus->addOrder(DieOrder(true));
}

void Planet::generateResources() {
	switch (m_type) {
	case PLANET_TYPE::GAS_GIANT:
	case PLANET_TYPE::ICE_GIANT:
		break;
	default:
		// Terrestrial planet
		
		Resource resource;
		if (Random::randBool()) {
			// 50% chance
			resource.type = "COMMON_ORE";
			resource.abundance = Random::randFloat(0.0f, 1.0f);
			m_resources.push_back(resource);
		}
		if (Random::randFloat(0.0f, 1.0f) < 0.25f) {
			// 25% chance
			resource.type = "UNCOMMON_ORE";
			resource.abundance = Random::randFloat(0.0f, 1.0f);
			m_resources.push_back(resource);
		}
		if (Random::randFloat(0.0, 1.0f) < 0.1f) {
			// 10% chance
			resource.type = "RARE_ORE";
			resource.abundance = Random::randFloat(0.0f, 1.0f);
			m_resources.push_back(resource);
		}
	}
}

void Planet::addEvent(const std::string& type) {
	PlanetEvent ev;
	ev.type = type;
	ev.timeSeconds = m_timeSinceCreaton;
	m_events.push_back(ev);
}

std::string Planet::getName(const Star* star) {
	return star->getName() + " " + Util::toRomanNumeral(star->getPlanetIndex(this) + 1);
}

bool Planet::hasResource(const std::string& resource) const {
	for (const Resource& r : m_resources) {
		if (r.type == resource) return true;
	}
	return false;
}

float Planet::getResourceAbundance(const std::string& type) const {
	for (const Resource& r : m_resources) {
		if (r.type == type) return r.abundance;
	}
	return 0.0f;
}

void Planet::setRadius(float radius) {
	m_shape.setSize(sf::Vector2f(radius * 2.0f, radius * 2.0f));
	m_shape.setOrigin(sf::Vector2f(getRadius(), getRadius()));
}

bool Planet::isMouseInRadius(const sf::RenderWindow& window, const Renderer& renderer) const {
	sf::Vector2f mouseWorldPos = renderer.mapPixelToCoords(sf::Mouse::getPosition(window));
	if (Math::distance(mouseWorldPos, m_shape.getPosition()) < getRadius()) {
		return true;
	}
	return false;
}

std::string Planet::getHabitabilityText() {
	float hab = getHabitability();
	if (hab < 0.1f) {
		return "Nightmare";
	}
	else if (hab < 0.25f) {
		return "Terrible";
	}
	else if (hab < 0.5f) {
		return "Challenging";
	}
	else if (hab < 0.9f) {
		return "Reasonable";
	}
	else if (hab < 1.5f) {
		return "Earthlike";
	}
	else {
		return "Earth";
	}
}

std::string Planet::getTemperatureText() {
	if (m_temperature < 200.0f) {
		return "Extremely Cold";
	}
	else if (m_temperature < 273.15f) {
		return "Frozen";
	}
	else if (m_temperature < 280.0f) {
		return "Cold";
	}
	else if (m_temperature < 295.0f) {
		return "Earthlike";
	}
	else if (m_temperature < 315.0f) {
		return "Hot";
	}
	else if (m_temperature < 373.15f) {
		return "Very Hot";
	}
	else if (m_temperature < 500.0f) {
		return "Boiling";
	}
	else {
		return "Melting";
	}
}

std::string Planet::getAtmosphereText() {
	if (m_atmosphere < 0.1f) {
		return "Nonexistent";
	}
	else if (m_atmosphere < 0.8f) {
		return "Thin";
	}
	else if (m_atmosphere < 1.1f) {
		return "Earthlike";
	}
	else if (m_atmosphere < 50.0f) {
		return "Heavy";
	}
	else {
		return "Crushing";
	}
}

std::string Planet::getWaterText() {
	if (m_water < 0.1f) {
		return "Devoid";
	}
	else if (m_water < 0.5f) {
		return "Dry";
	}
	else if (m_water < 0.8f) {
		return "Earthlike";
	}
	else {
		return "Water World";
	}
}

void Planet::discoverResource(const std::string& resourceType, sf::Vector2i pos, Faction* faction, Star& star) {
	Resource resource(resourceType);
	resource.abundance = 0.5f;
	resource.hidden = false;
	resource.pos = pos;

	m_resources.push_back(resource);

	if (faction != nullptr) {
		faction->addNewsEvent("We have discovered " + resource.getName() + " on " + getName(&star) + ".", sf::Color::Green);
	}
}