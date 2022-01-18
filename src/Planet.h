#pragma once

#include <SFML/Graphics.hpp>
#include "EffectsEmitter.h"
#include "Orbit.h"
#include "ParticleSystem.h"
#include "Identifiable.h"
#include "Colony.h"
#include "Resource.h"
#include "Weapon.h"

class Star;
class Faction;
class Projectile;
class Renderer;

class Planet : public Identifiable {
public:
	enum class PLANET_TYPE {
		UNKNOWN,
		DESERT,
		TERRA,
		BARREN,
		TUNDRA,
		GAS_GIANT,
		ICE_GIANT,
		TOXIC,
		LAVA,
		OCEAN,
		VOLCANIC
	};

	struct PlanetEvent {
		std::string type;
		float timeSeconds;

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& archive, const unsigned int version) {
			archive & type;
			archive & timeSeconds;
		}
	};
	
	Planet(sf::Vector2f pos, sf::Vector2f starPos, sf::Vector2f orbitPos, float starTemperature, bool moon = false);

	void draw(Renderer& renderer, sf::RenderWindow& window, Star* star, float time);
	void update(Star* currentStar, Faction* faction);
	void generateGasGiant(float baseTemperature);
	void generateTerrestrial(float baseTemperature);
	void onColonization();
	void createSpaceBus(sf::Color factionColor, Star* currentStar, Star* targetStar, Planet* targetPlanet, const std::string& type, int population, int maxPopulation);
	void generateResources();
	void addBombardProjectile(const Projectile& proj) { m_bombardProjectiles.push_back(proj); }
	void addEvent(const std::string& type);
	void setMoonOf(int planetIndex) { m_parentPlanetIndex = planetIndex; }
	void setRadius(float radius);

	float getTemperature() const { return m_temperature; }
	float getAtmosphericPressure() const { return m_atmosphere; }
	float getWater() const { return m_water; }
	float getRadius() const { return m_shape.getLocalBounds().width / 2.0; }
	float getHabitability() const;
	float getTimeSinceCreation() const { return m_timeSinceCreaton; }
	float getResourceAbundance(const std::string& type) const;

	Colony& getColony() { return m_colony; }

	bool isGasGiant() const { return m_gasGiant; }
	bool hasResource(const std::string& resource) const;
	bool isMoon() const { return m_parentPlanetIndex != -1; }
	bool isMouseInRadius(const sf::RenderWindow & window, const Renderer & renderer) const;

	sf::Vector2f getPos() const { return m_shape.getPosition(); }

	PLANET_TYPE getType() const { return m_type; }
	std::string getTypeString() const;
	std::vector<Resource>& getResources() { return m_resources; }
	const std::vector<PlanetEvent>& getEvents() const { return m_events; }
	std::string getName(const Star* star);
	std::string getHabitabilityText();
	std::string getTemperatureText();
	std::string getAtmosphereText();
	std::string getWaterText();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & boost::serialization::base_object<Identifiable>(*this);
		archive & m_shape;
		archive & m_shaderRandomSeed;
		archive & m_temperature;
		archive & m_atmosphere;
		archive & m_gasGiant;
		archive & m_water;
		archive & m_orbit;
		archive & m_type;
		archive & m_colony;
		archive & m_resources;
		archive & m_bombardProjectiles;
		archive & m_events;
		archive & m_timeSinceCreaton;
		archive & m_rings;
		archive & m_parentPlanetIndex;
	}
	
	Planet() {}

	sf::RectangleShape m_shape;
	
	float m_shaderRandomSeed = 1.0f;
	float m_temperature = 500.0f; // Kelvin
	float m_atmosphere = 1.0f; // Atmospheric pressure in Earth atmospheres
	float m_water = 0.0f; // Percent of planet covered in water from 0-1
	float m_timeSinceCreaton = 0.0f; // Seconds

	int m_parentPlanetIndex = -1;
	
	bool m_gasGiant = false;
	bool m_rings = false;

	Orbit m_orbit;
	PLANET_TYPE m_type = PLANET_TYPE::UNKNOWN;
	Colony m_colony;

	std::vector<Resource> m_resources;
	std::vector<Projectile> m_bombardProjectiles;
	std::vector<PlanetEvent> m_events;
};
