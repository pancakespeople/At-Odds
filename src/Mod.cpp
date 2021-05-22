#include "gamepch.h"
#include "Mod.h"
#include "Unit.h"
#include "Star.h"
#include "Faction.h"

BOOST_CLASS_EXPORT_GUID(FactoryMod, "FactoryMod")

void FactoryMod::update(Unit* unit, Star* currentStar, Faction& faction) {
	if (m_ticksToNextShip <= 0) {
		sf::Vector2f pos = unit->getPos();

		if (m_numShips % 10 == 0 && m_numShips != 0) {
			faction.addSpaceship(currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::DESTROYER_1, pos, currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor())));
			faction.addSpaceship(currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::CONSTRUCTION_SHIP, pos, currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor())));
		}
		else {
			faction.addSpaceship(currentStar->createSpaceship(std::make_unique<Spaceship>(
				Spaceship::SPACESHIP_TYPE::FRIGATE_1, pos, currentStar, unit->getAllegiance(), unit->getCollider().getOutlineColor())));
		}

		m_numShips++;
		m_ticksToNextShip = 2000;
	}
	else {
		m_ticksToNextShip -= 1;
	}
}