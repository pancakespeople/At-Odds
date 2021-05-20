#include "gamepch.h"
#include "Brain.h"
#include "Faction.h"
#include "Spaceship.h"
#include "Order.h"
#include "Constellation.h"
#include "Random.h"
#include "Building.h"

void Brain::onStart(Faction* faction) {
	
}

void Brain::controlFaction(Faction* faction) {
	if (m_state == AI_STATE::NONE || m_stateChangeTimer <= 0) {
		considerChangingState();
	}
	else if (m_state == AI_STATE::ATTACKING) {
		considerAttack(faction);
	}
	else if (m_state == AI_STATE::FORTIFYING) {
		considerFortifying(faction);
	}

	m_stateChangeTimer--;
}

void Brain::considerChangingState() {
	if (Random::randFloat(0.0f, 1.0f) < m_personality.aggressiveness) {
		m_state = AI_STATE::ATTACKING;
	}
	else {
		m_state = AI_STATE::FORTIFYING;
	}

	m_stateChangeTimer = 3000;
}

void Brain::onSpawn(Faction* faction) {

}

void Brain::considerFortifying(Faction* faction) {
	if (m_fortifyingVars.fortifyingTimer == 0) {
		for (Star* star : faction->getOwnedStars()) {
			if (star->numAlliedBuildings(faction->getID()) == 0 && faction->numIdleConstructionShips() > 0) {
				// Build outpost

				std::unique_ptr<Building> building = std::make_unique<Building>(
					Building::BUILDING_TYPE::OUTPOST, star, star->getRandomLocalPos(-10000.0f, 10000.0f), faction->getID(), faction->getColor(), false);
				Building* realBuilding = star->createBuilding(building);

				// Give idle construction ships order to build it
				faction->orderConstructionShipsBuild(realBuilding, true);

				AI_DEBUG_PRINT("Building outpost");
			}
			else {
				for (auto& building : star->getBuildings()) {
					// Build any unbuilt buildings
					if (building->getAllegiance() == faction->getID() && !building->isBuilt()) {
						faction->orderConstructionShipsBuild(building.get(), true, true);
						AI_DEBUG_PRINT("Ordering build of unbuilt building");
					}
				}

				if (Random::randBool() && faction->numUnbuiltBuildings(star) == 0 && faction->numIdleConstructionShips() > 0) {
					// Build turrets around jump points

					std::vector<JumpPoint>& jumpPoints = star->getJumpPoints();

					int numTurrets = Random::randInt(0, 4);
					int randJumpPointIndex = Random::randInt(0, jumpPoints.size() - 1);

					JumpPoint& point = jumpPoints[randJumpPointIndex];

					// Pick the first idle construction ship found to build the turrets
					Spaceship* conShip = faction->getConstructionShips(true)[0];

					for (int i = 0; i < numTurrets; i++) {
						sf::Vector2f pos = point.getPos() + Random::randVec(-2500.0f, 2500.0f);
						int rnd = Random::randInt(0, 4);

						Building* turret = nullptr;
						Building::BUILDING_TYPE type = Building::BUILDING_TYPE::LASER_TURRET;

						// Choose type
						if (rnd < 4) {
							if (Random::randBool()) {
								type = Building::BUILDING_TYPE::LASER_TURRET;
							}
							else {
								type = Building::BUILDING_TYPE::MACHINE_GUN_TURRET;
							}
						}
						else {
							type = Building::BUILDING_TYPE::GAUSS_TURRET;
						}

						// Create turret
						std::unique_ptr<Building> building = std::make_unique<Building>(type, star, pos, faction->getID(), faction->getColor(), false);
						turret = star->createBuilding(building);

						// Order turret to be built
						conShip->addOrder(InteractWithBuildingOrder(turret));
					}

					AI_DEBUG_PRINT("Building " << numTurrets << " turrets");
				}
			}
		}
		m_fortifyingVars.fortifyingTimer = 1600;
	}

	m_fortifyingVars.fortifyingTimer--;
}

void Brain::considerAttack(Faction* faction) {
	if (m_attackVars.expansionTarget == nullptr) {

		// Secure stars connected to capitol
		for (Star* s : faction->getCapitol()->getConnectedStars()) {
			if (s->getAllegiance() != faction->getID()) {
				m_attackVars.expansionTarget = s;
				//AI_DEBUG_PRINT("Expansion target (near capitol) chosen");
				break;
			}
		}

		if (m_attackVars.expansionTarget == nullptr) {
			std::vector<Star*>& ownedStars = faction->getOwnedStars();
			for (int i = faction->getOwnedStars().size() - 1; i > 0; i--) {
				for (Star* adj : ownedStars[i]->getConnectedStars()) {
					if (adj->getAllegiance() != faction->getID()) {
						m_attackVars.expansionTarget = adj;
						//AI_DEBUG_PRINT("Expansion target chosen");
						break;
					}
				}
				if (m_attackVars.expansionTarget != nullptr) {
					break;
				}
			}
		}

	}
	else {
		if (!m_attackVars.launchingAttack) {
			faction->giveAllCombatShipsOrder(TravelOrder(m_attackVars.expansionTarget));
			m_attackVars.launchingAttack = true;
			m_attackVars.attackTimer = 1600;
			//AI_DEBUG_PRINT("Begun attack");
		}
		else {
			if (m_attackVars.attackTimer == 0) {
				if (m_attackVars.expansionTarget->getAllegiance() == faction->getID()) {
					m_attackVars.expansionTarget = nullptr;
					m_attackVars.launchingAttack = false;
					//AI_DEBUG_PRINT("Capture of star complete");
				}
				else {
					m_attackVars.attackTimer = 1600;

					if (m_attackVars.expansionTarget->numAlliedShips(faction->getID()) == 0) {
						m_attackVars.attackFrustration++;
					}
					else {
						m_attackVars.attackFrustration = 0;
					}

					if (m_attackVars.attackFrustration >= 5) {
						m_attackVars.expansionTarget = nullptr;
						m_attackVars.launchingAttack = false;
						m_attackVars.attackFrustration = 0;
						//AI_DEBUG_PRINT("Gave up on capture of star");
					}
				}
			}
			else {
				m_attackVars.attackTimer--;
			}
		}
	}
}