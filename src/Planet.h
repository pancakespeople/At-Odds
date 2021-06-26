#pragma once

#include <SFML/Graphics.hpp>
#include "EffectsEmitter.h"
#include "Orbit.h"
#include "ParticleSystem.h"
#include "Identifiable.h"

class Star;
class Faction;

struct PlanetResource {
	enum class RESOURCE_TYPE {
		COMMON_ORE,
		UNCOMMON_ORE,
		RARE_ORE
	};

	RESOURCE_TYPE type = RESOURCE_TYPE::COMMON_ORE;
	float abundance = 0.0f; // Value between 0 and 1

	std::string getTypeString();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & type;
		archive & abundance;
	}
};

struct Colony {
	const static int growthTicks = 1000;
	
	int population = 0;
	int ticksUntilNextGrowth = growthTicks;
	int ticksToNextBus = 500;
	int ticksToNextResourceExploit = 1000;
	int allegiance = -1;

	float getGrowthRate(float planetHabitability);
	bool isColonizationLegal(int allegiance);
	void setFactionColonyLegality(int allegiance, bool legality);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & population;
		archive & ticksUntilNextGrowth;
		archive & ticksToNextBus;
		archive & allegiance;
		archive & m_factionColonyLegality;
	}
	
	std::unordered_map<int, bool> m_factionColonyLegality;
};

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
		LAVA
	};
	
	Planet(sf::Vector2f pos, sf::Vector2f starPos, float starTemperature);

	void draw(sf::RenderWindow& window, EffectsEmitter& emitter, float time);
	void update(Star* currentStar, Faction* faction);
	void updateColony(Star* currentStar, Faction* faction);
	void generateGasGiant(float baseTemperature);
	void generateTerrestrial(float baseTemperature);
	void onColonization();
	void createSpaceBus(sf::Color factionColor, Star* currentStar, Star* targetStar, Planet* targetPlanet);
	void generateResources();

	float getTemperature() const { return m_temperature; }
	float getAtmosphericPressure() const { return m_atmosphere; }
	float getWater() const { return m_water; }
	float getRadius() const { return m_shape.getRadius(); }
	float getHabitability() const;

	Colony& getColony() { return m_colony; }

	bool isGasGiant() const { return m_gasGiant; }

	sf::Vector2f getPos() const { return m_shape.getPosition(); }

	PLANET_TYPE getType() const { return m_type; }
	std::string getTypeString() const;
	std::vector<PlanetResource>& getResources() { return m_resources; }


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
	}
	
	Planet() {}

	sf::CircleShape m_shape;
	
	float m_shaderRandomSeed = 1.0f;
	float m_temperature = 500.0f; // Kelvin
	float m_atmosphere = 1.0f; // Atmospheric pressure in Earth atmospheres
	float m_water = 0.0f; // Percent of planet covered in water from 0-1
	
	bool m_gasGiant = false;

	Orbit m_orbit;
	PLANET_TYPE m_type = PLANET_TYPE::UNKNOWN;
	Colony m_colony;

	std::vector<PlanetResource> m_resources;
};
