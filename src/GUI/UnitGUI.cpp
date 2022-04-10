#include "gamepch.h"
#include "UnitGUI.h"
#include "../GameState.h"
#include "../Star.h"
#include "../Math.h"
#include "../Pathfinder.h"
#include "../Hyperlane.h"
#include "MinimapGUI.h"
#include "../Renderer.h"
#include "../AllianceList.h"
#include "../Constellation.h"
#include "../Util.h"
#include "../Keybindings.h"

UnitGUI::UnitGUI() {
	m_mouseSelectionBox.setFillColor(sf::Color(150.0f, 150.0f, 150.0f, 100.0f));
}

void UnitGUI::open(tgui::Gui& gui, Player& player) {
	m_selectedShips.clear();

	m_panel = tgui::Panel::create();
	m_panel->setPosition("60%", "55%");
	m_panel->setSize("40%", "5%");
	m_panel->getRenderer()->setOpacity(0.75f);
	m_panel->setVisible(false);
	gui.add(m_panel);

	m_label = tgui::Label::create();
	m_label->setOrigin(0.5f, 0.5f);
	m_label->setPosition("50%", "50%");
	m_label->setAutoSize(true);
	m_panel->add(m_label, "label");
		
	m_possessButton = GUI::Button::create("Possess");
	m_possessButton->setOrigin(0.0f, 0.5f);
	m_possessButton->setPosition("0%", "50%");
	m_panel->add(m_possessButton);

	m_possessButton->onClick([this, &player]() {
		if (player.getControlledShip() != m_selectedShips[0]) {
			player.setControlledShip(m_selectedShips[0]);
			m_possessButton->setText("Depossess");
		}
		else {
			player.setControlledShip(nullptr);
			m_possessButton->setText("Possess");
		}
	});

	m_panel->setVisible(false);
	updatePanel();
}

void UnitGUI::update(const sf::RenderWindow& window, Renderer& renderer, Star* currentStar, Player& player, tgui::Panel::Ptr mainPanel, MinimapGUI& minimap) {
	// Remove dead or unselected stuff
	cleanUpDeadStuff();
	updatePanel();

	sf::FloatRect minimapViewport = minimap.getViewport();
	sf::Vector2f pos = sf::Vector2f(minimapViewport.left, minimapViewport.top - 0.05f);
	sf::Vector2f size = sf::Vector2f(minimapViewport.width, 0.05f);

	if (m_panel->getPosition().x != pos.x * window.getSize().x || m_panel->getPosition().y != pos.y * window.getSize().y) {
		tgui::String posLayoutX = std::to_string(pos.x * 100.0f) + "%";
		tgui::String posLayoutY = std::to_string(pos.y * 100.0f) + "%";
		tgui::String sizeLayoutX = std::to_string(size.x * 100.0f) + "%";
		tgui::String sizeLayoutY = std::to_string(size.y * 100.0f) + "%";

		m_panel->setPosition(posLayoutX, posLayoutY);
		m_panel->setSize(sizeLayoutX, sizeLayoutY);
	}

	if (m_selectedShips.size() > 0) {
		if (player.getControlledShip() != m_selectedShips[0] || m_selectedShips.size() > 1) {
			if (m_possessButton->getText() != "Possess") {
				m_possessButton->setText("Possess");
			}

			if (player.getControlledShip() != nullptr) {
				player.setControlledShip(nullptr);
			}
		}
	}
	else {
		if (player.getControlledShip() != nullptr) {
			player.setControlledShip(nullptr);
		}
	}
}

void UnitGUI::updatePanel() {
	std::string text;
	if (m_selectedShips.size() > 1) {
		m_panel->setVisible(true);
		text = std::to_string(m_selectedShips.size()) + "x " + "selected";
		m_possessButton->setVisible(false);
	}
	else if (m_selectedShips.size() == 1) {
		m_panel->setVisible(true);
		text = "Selected: " + m_selectedShips.front()->getName();

		if (m_selectedShips[0]->canPlayerGiveOrders() && m_selectedShips[0]->canReceiveOrders()) {
			m_possessButton->setVisible(true);
		}
		else {
			m_possessButton->setVisible(false);
		}
	}
	else if (m_selectedAsteroid != nullptr) {
		m_panel->setVisible(true);
		m_possessButton->setVisible(false);
		text += "Resource: " + Resource(m_selectedAsteroid->getResource()).getName() + "\n";
		text += "Remaining: " + Util::cutOffDecimal(m_selectedAsteroid->getResourceCount(), 2);
	}
	else {
		m_panel->setVisible(false);
		if (m_possessButton->getText() != "Possess") {
			m_possessButton->setText("Possess");
		}
	}

	if (m_label->getText() != text) {
		m_label->setText(text);
	}
}

void UnitGUI::draw(sf::RenderWindow& window) {
	if (m_mouseDown) {
		sf::View oldView = window.getView();
		window.setView(window.getDefaultView());

		window.draw(m_mouseSelectionBox);

		window.setView(oldView);
	}
}

void UnitGUI::onEvent(const sf::Event& ev, sf::RenderWindow& window, Renderer& renderer, GameState& state, Constellation& constellation, tgui::Panel::Ptr mainPanel, MinimapGUI& minimap) {
	if (state.getLocalViewStar() != nullptr) {
		if (Keybindings::isKeyPress("SelectAllCombatUnitsInSystem", ev)) {
			deselectAll();

			for (auto& ship : state.getLocalViewStar()->getSpaceships()) {
				if (ship->getAllegiance() == state.getPlayer().getFaction()) {
					if (ship->isCombatShip()) {
						ship->onSelected();
						m_selectedShips.push_back(ship.get());
					}
				}
			}
		}
	}

	bool mainPanelFocused = true;
	if (mainPanel != nullptr) {
		mainPanelFocused = mainPanel->isFocused();
	}

	if (!mainPanelFocused || m_disabled) {
		return;
	}

	if (ev.type == sf::Event::MouseMoved && m_mouseDown) {
		// Mouse held, change size of selection box

		sf::Vector2i mpos = { ev.mouseMove.x, ev.mouseMove.y };
		sf::Vector2f selectionOriginPos = m_mouseSelectionBox.getPosition();

		sf::Vector2f newSize = sf::Vector2f(mpos.x - selectionOriginPos.x, mpos.y - selectionOriginPos.y);
		m_mouseSelectionBox.setSize(newSize);
	}
	else if (ev.type == sf::Event::MouseButtonPressed && !minimap.isMouseInMinimap(window)) {
		if (ev.mouseButton.button == sf::Mouse::Left) {
			// Mouse begins to be held down, set position of selection box

			m_mouseSelectionBox.setPosition(sf::Vector2f(ev.mouseButton.x, ev.mouseButton.y));
			m_mouseDown = true;
		}
	}
	else if (ev.type == sf::Event::MouseButtonReleased) {
		if (ev.mouseButton.button == sf::Mouse::Left) {
			// Mouse released, select units

			onSelect(renderer, state.getLocalViewStar(), state.getPlayer().getFaction());
			onMouseClick(ev, renderer, state.getLocalViewStar(), state.getPlayer().getFaction());
			m_mouseDown = false;
		}
	}

	// Right click events
	if (state.getState() == GameState::State::LOCAL_VIEW && mainPanelFocused) {
		if (ev.type == sf::Event::MouseButtonPressed) {
			if (ev.mouseButton.button == sf::Mouse::Right) {
				onRightClickLocalView(window, renderer, constellation, state);
			}
		}
	}
	else if (state.getState() == GameState::State::WORLD_VIEW) {
		if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Right) {
			onRightClickWorldView(window, renderer, constellation);
		}
	}
}

void UnitGUI::onRightClickWorldView(sf::RenderWindow & window, Renderer & renderer, Constellation & constellation) {
	sf::Vector2i screenPos = sf::Mouse::getPosition(window);
	sf::Vector2f worldClick = renderer.mapPixelToCoords(screenPos);
	Star* star = nullptr;

	// Check if click on star
	for (std::unique_ptr<Star>& s : constellation.getStars()) {
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

void UnitGUI::onRightClickLocalView(sf::RenderWindow & window, Renderer & renderer, Constellation & constellation, GameState & state) {
	// Give orders to selected ships

	if (m_selectedShips.size() > 0) {
		sf::Vector2i screenPos = sf::Mouse::getPosition(window);
		sf::Vector2f worldClick = renderer.mapPixelToCoords(screenPos);
		JumpPoint* jumpPoint = nullptr;
		Spaceship* attackTarget = nullptr;
		Building* buildingClick = nullptr;
		Planet* planetClick = nullptr;
		Asteroid* asteroidClick = nullptr;
		const AllianceList& alliances = constellation.getAlliances();

		// Check if click was on a jump point
		for (JumpPoint& j : state.getLocalViewStar()->getJumpPoints()) {
			if (j.isPointInRadius(worldClick)) {
				jumpPoint = &j;
				break;
			}
		}

		// Check if click was on an enemy ship
		for (auto& s : state.getLocalViewStar()->getSpaceships()) {
			if (!alliances.isAllied(s->getAllegiance(), m_selectedShips[0]->getAllegiance())) {
				if (s->getCollider().contains(worldClick)) {
					attackTarget = s.get();
					break;
				}
			}
		}

		// Check if click was on a building
		for (auto& building : state.getLocalViewStar()->getBuildings()) {
			if (building->getCollider().contains(worldClick)) {
				buildingClick = building.get();
			}
		}

		// Planets
		for (auto& planet : state.getLocalViewStar()->getPlanets()) {
			if (Math::distance(worldClick, planet.getPos()) < planet.getRadius()) {
				planetClick = &planet;
			}
		}

		// Asteroids
		for (auto& asteroid : state.getLocalViewStar()->getAsteroids()) {
			if (Math::distance(worldClick, asteroid.getPos()) < asteroid.getRadius()) {
				asteroidClick = &asteroid;
			}
		}

		int index = 0;
		sf::Vector2f avgPos = getAveragePosOfSelectedShips();
		// Add orders
		for (Spaceship* s : m_selectedShips) {
			if (!s->canPlayerGiveOrders()) continue;
			if (state.getLocalViewStar() == s->getCurrentStar()) {
				if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) s->clearOrders();
				if (attackTarget != nullptr) {
					s->addOrder(AttackOrder(attackTarget));
				}
				else if (buildingClick != nullptr) {
					if (!alliances.isAllied(buildingClick->getAllegiance(), s->getAllegiance())) {
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
				else if (asteroidClick != nullptr) {
					s->addOrder(MineAsteroidOrder(asteroidClick));
				}
				else {
					float angle = Math::angleBetween(avgPos, worldClick);
					float perpendicularAngle = angle - 90.0f;
					sf::Vector2f formationMove(std::cos(perpendicularAngle) * index * 100.0f, std::sin(perpendicularAngle) * index * 100.0f);

					s->addOrder(FlyToOrder(worldClick + formationMove));
				}
			}
			index++;
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

sf::Vector2f UnitGUI::getAveragePosOfSelectedShips() {
	sf::Vector2f pos;
	for (Spaceship* ship : m_selectedShips) {
		if (ship->canPlayerGiveOrders()) {
			pos += ship->getPos();
		}
	}
	pos = sf::Vector2f(pos.x / m_selectedShips.size(), pos.y / m_selectedShips.size());
	return pos;
}

void UnitGUI::onSelect(const Renderer& renderer, Star* star, int playerAllegiance) {
	bool allowCivilianShips = true;

	m_selecting = false;
	deselectAll();

	if (star != nullptr) {

		// Select spaceships
		for (auto& s : star->getSpaceships()) {
			if (s->getCurrentStar()->isLocalViewActive() && s->getAllegiance() == playerAllegiance) {

				sf::Vector2i screenPos = renderer.mapCoordsToPixel(s->getPos());
				sf::FloatRect selection = m_mouseSelectionBox.getGlobalBounds();

				if (screenPos.x >= selection.left && screenPos.x <= selection.left + selection.width &&
					screenPos.y >= selection.top && screenPos.y <= selection.top + selection.height) {
					if (s->isCombatShip()) {
						allowCivilianShips = false;
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
		for (auto& building : star->getBuildings()) {
			if (building->isSelected()) building->onDeselected();
		}

		// Deselect asteroid
		if (m_selectedAsteroid != nullptr) {
			m_selectedAsteroid->setSelected(false);
			m_selectedAsteroid = nullptr;
		}

		// Don't mix combat and civilian ships - delete from container
		if (!allowCivilianShips) {
			auto it = std::remove_if(m_selectedShips.begin(), m_selectedShips.end(), [](Spaceship* ship) {
				if (!ship->isCombatShip()) {
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

void UnitGUI::onMouseClick(const sf::Event& ev, const Renderer& renderer, Star* currentStar, int playerFaction) {
	// Select an individual unit

	if (currentStar != nullptr) {

		sf::Vector2i screenPos = { ev.mouseButton.x, ev.mouseButton.y };
		sf::Vector2f worldClick = renderer.mapPixelToCoords(screenPos);

		for (auto& s : currentStar->getSpaceships()) {
			if (s->getCollider().contains(worldClick)) {
				if (s->getAllegiance() == playerFaction) {
					m_selectedShips.push_back(s.get());
					s->onSelected();
					return;
				}
			}
		}

		for (auto& building : currentStar->getBuildings()) {
			if (building->getCollider().contains(worldClick) && building->getAllegiance() == playerFaction) {
				m_selectedBuildings.push_back(building.get());
				building->onSelected();
				return;
			}
		}

		for (auto& asteroid : currentStar->getAsteroids()) {
			if (asteroid.contains(worldClick)) {
				m_selectedAsteroid = &asteroid;
				asteroid.setSelected(true);
				return;
			}
		}

	}
}

void UnitGUI::cleanUpDeadStuff() {
	m_selectedShips.erase(std::remove_if(m_selectedShips.begin(), m_selectedShips.end(), [](Spaceship* s) {return s->isDead() || !s->isSelected(); }), m_selectedShips.end());
	m_selectedBuildings.erase(std::remove_if(m_selectedBuildings.begin(), m_selectedBuildings.end(), [](Building* b) {return b->isDead() || !b->isSelected(); }), m_selectedBuildings.end());

	if (m_selectedAsteroid != nullptr) {
		if (m_selectedAsteroid->isDestructing()) {
			m_selectedAsteroid->setSelected(false);
			m_selectedAsteroid = nullptr;
		}
	}
}

void UnitGUI::deselectAll() {
	for (Spaceship* ship : m_selectedShips) {
		ship->onDeselected();
	}
	m_selectedShips.clear();

	for (Building* building : m_selectedBuildings) {
		building->onDeselected();
	}
	m_selectedBuildings.clear();

	if (m_selectedAsteroid != nullptr) {
		m_selectedAsteroid->setSelected(false);
		m_selectedAsteroid = nullptr;
	}
}