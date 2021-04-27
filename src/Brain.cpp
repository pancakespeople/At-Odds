#include "gamepch.h"
#include "Brain.h"
#include "Faction.h"
#include "Spaceship.h"
#include "Order.h"
#include "Constellation.h"

void Brain::onStart(Faction* faction) {
	
}

void Brain::controlFaction(Faction* faction) {
	if (m_expansionTarget == nullptr) {
		
		// Secure stars connected to capitol
		for (Star* s : faction->getCapitol()->getConnectedStars()) {
			if (s->getAllegiance() != faction->getID()) {
				m_expansionTarget = s;
				AI_DEBUG_PRINT("Expansion target (near capitol) chosen");
				break;
			}
		}

		if (m_expansionTarget == nullptr) {
			std::vector<Star*>& ownedStars = faction->getOwnedStars();
			for (int i = faction->getOwnedStars().size() - 1; i > 0; i--) {
				for (Star* adj : ownedStars[i]->getConnectedStars()) {
					if (adj->getAllegiance() != faction->getID()) {
						m_expansionTarget = adj;
						AI_DEBUG_PRINT("Expansion target chosen");
						break;
					}
				}
				if (m_expansionTarget != nullptr) {
					break;
				}
			}
		}

	}
	else {
		if (!m_launchingAttack) {
			faction->giveAllShipsOrder(TravelOrder(m_expansionTarget));
			m_launchingAttack = true;
			m_attackTimer = 1600;
			AI_DEBUG_PRINT("Begun attack");
		}
		else {
			if (m_attackTimer == 0) {
				if (m_expansionTarget->getAllegiance() == faction->getID()) {
					m_expansionTarget = nullptr;
					m_launchingAttack = false;
					AI_DEBUG_PRINT("Capture of star complete");
				}
				else {
					m_attackTimer = 1600;
					
					if (m_expansionTarget->numAlliedShips(faction->getID()) == 0) {
						m_attackFrustration++;
					}
					else {
						m_attackFrustration = 0;
					}

					if (m_attackFrustration >= 5) {
						m_expansionTarget = nullptr;
						m_launchingAttack = false;
						m_attackFrustration = 0;
						AI_DEBUG_PRINT("Gave up on capture of star");
					}
				}
			}
			else {
				m_attackTimer--;
			}
		}
	}
}

void Brain::onSpawn(Faction* faction) {

}