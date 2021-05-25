#include "gamepch.h"
#include "Mod.h"
#include "Unit.h"
#include "Star.h"
#include "Faction.h"
#include "Random.h"

BOOST_CLASS_EXPORT_GUID(FactoryMod, "FactoryMod")

void FactoryMod::update(Unit* unit, Star* currentStar, Faction& faction) {
	if (m_ticksToNextShip <= 0) {
		sf::Vector2f pos = unit->getPos();
		float radius = unit->getCollider().getRadius();
		
		if (m_numShips % 10 == 0 && m_numShips != 0) {
			Spaceship* destroyer = currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::DESTROYER_1, pos + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor()));
			faction.addSpaceship(destroyer);

			Spaceship* constrShip = currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::CONSTRUCTION_SHIP, pos + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor()));
			faction.addSpaceship(constrShip);
		}
		else {
			Spaceship* frigate = currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::FRIGATE_1, pos + Random::randVec(-radius, radius), currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor()));
			faction.addSpaceship(frigate);
		}

		m_numShips++;
		m_ticksToNextShip = 2000;
	}
	else {
		m_ticksToNextShip -= 1;
	}
}