#include "gamepch.h"
#include "Mod.h"
#include "Unit.h"
#include "Star.h"
#include "Faction.h"
#include "Random.h"
#include "Order.h"
#include "Math.h"

BOOST_CLASS_EXPORT_GUID(FactoryMod, "FactoryMod")
BOOST_CLASS_EXPORT_GUID(FighterBayMod, "FighterBayMod")

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

FighterBayMod::FighterBayMod(const Unit* unit, Star* star, int allegiance, sf::Color color) {
	for (int i = 0; i < 4; i++) {
		auto ship = std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::FIGHTER, unit->getPos(), star, allegiance, color);
		Spaceship* shipPtr = star->createSpaceship(ship);
		shipPtr->disable();
		m_fighterShipIds.push_back(shipPtr->getID());
	}
}

void FighterBayMod::update(Unit* unit, Star* currentStar, Faction& faction) {
	if (unit->isDead()) {
		// Spit out fighters if unit died
		
		launchFighters(currentStar);
		return;
	}
	
	int numEnemyUnits = unit->findEnemyUnits().size();
	
	if (numEnemyUnits > 0 && m_fighterStatus == FIGHTER_STATUS::DOCKED) {
		// Enable fighters if enemies are in system
		
		launchFighters(currentStar);
	}
	
	if (numEnemyUnits == 0 && m_fighterStatus == FIGHTER_STATUS::FIGHTING) {
		//Recall fighters

		for (int i = 0; i < m_fighterShipIds.size(); i++) {
			Spaceship* fighter = currentStar->getShipByID(m_fighterShipIds[i]);
			if (fighter != nullptr) {
				fighter->clearOrders();
				fighter->addOrder(FlyToOrder(unit->getPos()));
			}
			else {
				m_fighterShipIds.erase(m_fighterShipIds.begin() + i);
				i--;
			}
		}

		m_fighterStatus = FIGHTER_STATUS::RETURNING;
	}
	
	if (m_fighterStatus == FIGHTER_STATUS::RETURNING) {
		int numReturningFighters = 0;
		
		for (int i = 0; i < m_fighterShipIds.size(); i++) {
			Spaceship* fighter = currentStar->getShipByID(m_fighterShipIds[i]);
			if (fighter != nullptr) {
				if (!fighter->isDisabled()) {
					if (Math::distance(fighter->getPos(), unit->getPos()) < unit->getCollider().getRadius()) {
						fighter->disable();
					}
					else {
						numReturningFighters++;
					}
				}
			}
			else {
				m_fighterShipIds.erase(m_fighterShipIds.begin() + i);
				i--;
			}
		}

		if (m_fighterShipIds.size() == 0 || numReturningFighters == 0) {
			m_fighterStatus = FIGHTER_STATUS::DOCKED;
			DEBUG_PRINT("fighters docked");
		}
	}
}

void FighterBayMod::launchFighters(Star* currentStar) {
	for (int i = 0; i < m_fighterShipIds.size(); i++) {
		Spaceship* fighter = currentStar->getShipByID(m_fighterShipIds[i]);
		if (fighter != nullptr) {
			fighter->enable();
		}
		else {
			m_fighterShipIds.erase(m_fighterShipIds.begin() + i);
			i--;
		}
	}

	m_fighterStatus = FIGHTER_STATUS::FIGHTING;
}