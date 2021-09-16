#include "gamepch.h"
#include "UnitGUI.h"
#include "../GameState.h"
#include "../Star.h"
#include "../Math.h"
#include "../Pathfinder.h"
#include "../Hyperlane.h"

UnitGUI::UnitGUI() {
	m_mouseSelectionBox.setFillColor(sf::Color(150.0f, 150.0f, 150.0f, 100.0f));
}

void UnitGUI::open(tgui::Gui& gui) {
	m_panel = tgui::Panel::create();
	m_panel->setPosition("90%", "90%");
	m_panel->setSize("7.5%", "5%");
	m_panel->getRenderer()->setOpacity(0.75f);
	m_panel->setVisible(false);
	gui.add(m_panel);

	m_label = tgui::Label::create();
	m_label->setSize("100%", "100%");
	m_label->setText(std::to_string(m_selectedShips.size()) + "x " + "selected");
	m_panel->add(m_label);
}

void UnitGUI::update(const sf::RenderWindow& window, Star* currentStar, int playerFaction, tgui::Panel::Ptr mainPanel) {
	static bool mouseHeld = false;

	// Remove dead or unselected stuff
	m_selectedShips.erase(std::remove_if(m_selectedShips.begin(), m_selectedShips.end(), [](Spaceship* s) {return s->isDead() || !s->isSelected(); }), m_selectedShips.end());
	m_selectedBuildings.erase(std::remove_if(m_selectedBuildings.begin(), m_selectedBuildings.end(), [](Building* b) {return b->isDead() || !b->isSelected(); }), m_selectedBuildings.end());

	if (!mainPanel->isFocused()) return;

	// Mouse begins to be held down - Begin selection
	if (!mouseHeld && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		sf::Vector2i mpos = sf::Mouse::getPosition(window);
		m_mouseSelectionBox.setPosition(sf::Vector2f(mpos.x, mpos.y));
	}

	// Mouse held - Create selection area
	else if (mouseHeld && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		sf::Vector2i mpos = sf::Mouse::getPosition(window);
		sf::Vector2f selectionOriginPos = m_mouseSelectionBox.getPosition();

		sf::Vector2f newSize = sf::Vector2f(mpos.x - selectionOriginPos.x, mpos.y - selectionOriginPos.y);
		m_mouseSelectionBox.setSize(newSize);
	}

	// Mouse let go - select ships
	else if (mouseHeld && !sf::Mouse::isButtonPressed(sf::Mouse::Left)
		&& std::abs(m_mouseSelectionBox.getSize().x) >= 5.0f
		&& std::abs(m_mouseSelectionBox.getSize().y) >= 5.0f) {
		bool allowConstructionShips = true;

		m_selectedShips.clear();

		if (currentStar != nullptr) {
			
			// Select spaceships
			for (auto& s : currentStar->getSpaceships()) {
				if (s->getCurrentStar()->isLocalViewActive() && s->getAllegiance() == playerFaction) {

					sf::Vector2i screenPos = window.mapCoordsToPixel(s->getPos());
					sf::FloatRect selection = m_mouseSelectionBox.getGlobalBounds();

					if (screenPos.x >= selection.left && screenPos.x <= selection.left + selection.width &&
						screenPos.y >= selection.top && screenPos.y <= selection.top + selection.height) {
						if (s->getConstructionSpeed() == 0.0f) {
							allowConstructionShips = false;
						}

						s->onSelected();
						m_selectedShips.push_back(s.get());
					}
					else if (s->isSelected()) {
						s->onDeselected();
					}
				}
			}

			// Deselect all buildings
			for (auto& building : currentStar->getBuildings()) {
				if (building->isSelected()) building->onDeselected();
			}

			// Don't mix combat and construction ships - delete from container
			if (!allowConstructionShips) {
				auto it = std::remove_if(m_selectedShips.begin(), m_selectedShips.end(), [](Spaceship* ship) {
					if (ship->getConstructionSpeed() > 0.0f) {
						ship->onDeselected();
						return true;
					}
					return false;
					});

				m_selectedShips.erase(it, m_selectedShips.end());
			}
		}

		m_mouseSelectionBox.setSize(sf::Vector2f(0.0f, 0.0f));
	}

	// Mouse click - deselect ships or select individual unit
	else if (mouseHeld && !sf::Mouse::isButtonPressed(sf::Mouse::Left) && m_mouseSelectionBox.getSize().x < 5.0f && m_mouseSelectionBox.getSize().y < 5.0f) {
		for (Spaceship* s : m_selectedShips) {
			if (s->isSelected()) {
				s->onDeselected();
			}
		}
		for (Building* b : m_selectedBuildings) {
			if (b->isSelected()) {
				b->onDeselected();
			}
		}
		m_selectedShips.clear();
		m_selectedBuildings.clear();

		// Select an individual unit

		if (currentStar != nullptr) {

			sf::Vector2i screenPos = sf::Mouse::getPosition(window);
			sf::Vector2f worldClick = window.mapPixelToCoords(screenPos);

			for (auto& s : currentStar->getSpaceships()) {
				if (s->getCollider().getGlobalBounds().contains(worldClick)) {
					if (s->getAllegiance() == playerFaction) {
						m_selectedShips.push_back(s.get());
						s->onSelected();
						break;
					}
				}
			}

			for (auto& building : currentStar->getBuildings()) {
				if (building->getCollider().getGlobalBounds().contains(worldClick) && building->getAllegiance() == playerFaction) {
					m_selectedBuildings.push_back(building.get());
					building->onSelected();
					break;
				}
			}

		}
	}

	mouseHeld = sf::Mouse::isButtonPressed(sf::Mouse::Left);

	if (m_selectedShips.size() > 0) {
		m_panel->setVisible(true);
		m_label->setText(std::to_string(m_selectedShips.size()) + "x " + "selected");
	}
	else {
		m_panel->setVisible(false);
	}
}

void UnitGUI::draw(sf::RenderWindow& window) {
	sf::View oldView = window.getView();
	window.setView(window.getDefaultView());

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
		window.draw(m_mouseSelectionBox);
	}

	window.setView(oldView);
}

void UnitGUI::onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state, std::vector<std::unique_ptr<Star>>& stars, tgui::Panel::Ptr mainPanel) {
	if (mainPanel != nullptr) {
		if (!mainPanel->isFocused()) return;
	}

	if (state.getState() == GameState::State::LOCAL_VIEW) {
		if (ev.type == sf::Event::MouseButtonPressed) {
			if (ev.mouseButton.button == sf::Mouse::Right) {
				// Give orders to selected ships

				if (m_selectedShips.size() > 0) {
					sf::Vector2i screenPos = sf::Mouse::getPosition(window);
					sf::Vector2f worldClick = window.mapPixelToCoords(screenPos);
					JumpPoint* jumpPoint = nullptr;
					Spaceship* attackTarget = nullptr;
					Building* buildingClick = nullptr;
					Planet* planetClick = nullptr;

					// Check if click was on a jump point
					for (JumpPoint& j : state.getLocalViewStar()->getJumpPoints()) {
						if (j.isPointInRadius(worldClick)) {
							jumpPoint = &j;
							break;
						}
					}

					// Check if click was on an enemy ship
					for (auto& s : state.getLocalViewStar()->getSpaceships()) {
						if (s->getAllegiance() != m_selectedShips[0]->getAllegiance()) {
							if (s->getCollider().getGlobalBounds().contains(worldClick)) {
								attackTarget = s.get();
								break;
							}
						}
					}

					// Check if click was on a building
					for (auto& building : state.getLocalViewStar()->getBuildings()) {
						if (building->getCollider().getGlobalBounds().contains(worldClick)) {
							buildingClick = building.get();
						}
					}

					// Planets
					for (auto& planet : state.getLocalViewStar()->getPlanets()) {
						if (Math::distance(worldClick, planet.getPos()) < planet.getRadius()) {
							planetClick = &planet;
						}
					}

					// Add orders
					for (Spaceship* s : m_selectedShips) {
						if (!s->canPlayerGiveOrders()) continue;
						if (state.getLocalViewStar() == s->getCurrentStar()) {
							if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) s->clearOrders();
							if (attackTarget != nullptr) {
								s->addOrder(AttackOrder(attackTarget));
							}
							else if (buildingClick != nullptr) {
								if (buildingClick->getAllegiance() != s->getAllegiance()) {
									s->addOrder(AttackOrder(buildingClick)); // Attack enemy building
								}
								else {
									s->addOrder(InteractWithBuildingOrder(buildingClick)); // Interact with friendly building
								}
							}
							else if (jumpPoint != nullptr) {
								s->addOrder(JumpOrder(jumpPoint));
							}
							else if (planetClick != nullptr) {
								s->addOrder(InteractWithPlanetOrder(planetClick, state.getLocalViewStar()));
							}
							else {
								s->addOrder(FlyToOrder(worldClick));
							}
						}
					}
				}
			}
		}
	}
	else if (state.getState() == GameState::State::WORLD_VIEW) {
		if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Right) {
			sf::Vector2i screenPos = sf::Mouse::getPosition(window);
			sf::Vector2f worldClick = window.mapPixelToCoords(screenPos);
			Star* star = nullptr;

			// Check if click on star
			for (std::unique_ptr<Star>& s : stars) {
				if (s->isInShapeRadius(worldClick.x, worldClick.y)) {
					star = s.get();
					break;
				}
			}

			// Add travel order to star
			if (star != nullptr) {
				for (Spaceship* s : m_selectedShips) {
					if (!s->canPlayerGiveOrders()) continue;
					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) s->clearOrders();
					s->addOrder(TravelOrder(star));
				}
				if (m_selectedShips.size() > 0) {
					drawStarPath(m_selectedShips[0]->getCurrentStar(), star);
				}
			}
		}
	}
}

void UnitGUI::drawStarPath(Star* begin, Star* end) {
	std::list<Star*> path = Pathfinder::findPath(begin, end);

	while (path.size() > 1) {
		Star* frontStar = path.front();
		path.pop_front();

		for (JumpPoint& j : frontStar->getJumpPoints()) {
			if (j.getConnectedOtherStar() == path.front()) {
				j.getHyperlane()->enablePathEffect();
			}
		}
	}
}