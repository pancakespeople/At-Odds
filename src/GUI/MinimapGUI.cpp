#include "gamepch.h"
#include "MinimapGUI.h"
#include "../Star.h"
#include "../Math.h"
#include "UnitGUI.h"
#include "../AllianceList.h"
#include "../Keybindings.h"

void MinimapGUI::draw(sf::RenderWindow& window, Star* currentStar, int playerAllegiance, Camera& camera, const AllianceList& alliances) {
	if (currentStar != nullptr && !m_hidden) {
		sf::View oldView = window.getView();
		float ratio = oldView.getSize().x / oldView.getSize().y;

		float mapRadius = 50000.0f;
		if (currentStar->getPlanets().size() > 0) {
			Planet* furthestPlanet = &currentStar->getPlanets().back();
			if (furthestPlanet->isMoon()) {
				furthestPlanet = &currentStar->getPlanets()[currentStar->getPlanets().size() - 2];
			}

			mapRadius = Math::distance(furthestPlanet->getPos(), currentStar->getLocalViewCenter()) * 2.0f + 100.0f;
		}

		m_view.setCenter(currentStar->getPos());
		m_view.setSize(mapRadius, mapRadius);
		m_view.setViewport(sf::FloatRect(1.0f - 0.4f / ratio, 0.6f, 0.4f / ratio, 0.4f));

		window.setView(m_view);

		m_minimapCircle.setFillColor(sf::Color(125, 125, 125, 125));
		m_minimapCircle.setPosition(currentStar->getPos());
		m_minimapCircle.setRadius(mapRadius / 2.0f);
		m_minimapCircle.setOrigin(mapRadius / 2.0f, mapRadius / 2.0f);
		m_minimapCircle.setPointCount(100);
		window.draw(m_minimapCircle);

		sf::FloatRect cameraRect = camera.getRect();
		std::array<sf::Vertex, 5> rectVertices;
		rectVertices[0] = sf::Vertex{ sf::Vector2f(cameraRect.left, cameraRect.top) };
		rectVertices[1] = sf::Vertex{ sf::Vector2f(cameraRect.left + cameraRect.width, cameraRect.top) };
		rectVertices[2] = sf::Vertex{ sf::Vector2f(cameraRect.left + cameraRect.width, cameraRect.top + cameraRect.height) };
		rectVertices[3] = sf::Vertex{ sf::Vector2f(cameraRect.left, cameraRect.top + cameraRect.height) };
		rectVertices[4] = sf::Vertex{ sf::Vector2f(cameraRect.left, cameraRect.top) };
		window.draw(&rectVertices[0], rectVertices.size(), sf::PrimitiveType::LineStrip);

		sf::CircleShape dot;
		dot.setFillColor(sf::Color::Yellow);
		dot.setPosition(currentStar->getPos());
		dot.setRadius(mapRadius / 100.0f);
		dot.setOrigin(mapRadius / 100.0f, mapRadius / 100.0f);

		window.draw(dot);

		//sf::RectangleShape rect;
		//rect.setSize(sf::Vector2f(mapRadius / 100.0f, mapRadius / 100.0f));
		//rect.setOrigin(mapRadius / 200.0f, mapRadius / 200.0f);

		if ((currentStar->isDrawingHidden() && currentStar->isDiscovered(playerAllegiance)) || playerAllegiance == -1) {
			
			// Ships
			dot.setPointCount(3);

			for (auto& ship : currentStar->getSpaceships()) {
				if (Math::distance(ship->getPos(), currentStar->getLocalViewCenter()) < m_minimapCircle.getRadius()) {
					dot.setPosition(ship->getPos());
					dot.setRotation(-ship->getFacingAngle() + 90.0f);

					if (playerAllegiance == -1) {
						dot.setFillColor(ship->getFactionColor());
					}
					else {
						if (ship->getAllegiance() == playerAllegiance) {
							dot.setFillColor(sf::Color::Green);
						}
						else if (alliances.isAllied(ship->getAllegiance(), playerAllegiance)) {
							dot.setFillColor(sf::Color::Blue);
						}
						else {
							dot.setFillColor(sf::Color::Red);
						}
					}

					window.draw(dot);
				}
			}

			// Buildings
			dot.setPointCount(4);
			dot.setRotation(45.0f);

			for (auto& building : currentStar->getBuildings()) {
				if (Math::distance(building->getPos(), currentStar->getLocalViewCenter()) < m_minimapCircle.getRadius()) {
					dot.setPosition(building->getPos());

					if (playerAllegiance == -1) {
						dot.setFillColor(building->getFactionColor());
					}
					else {
						if (building->getAllegiance() == playerAllegiance) {
							dot.setFillColor(sf::Color::Green);
						}
						else if (alliances.isAllied(building->getAllegiance(), playerAllegiance)) {
							dot.setFillColor(sf::Color::Blue);
						}
						else {
							dot.setFillColor(sf::Color::Red);
						}
					}

					window.draw(dot);
				}
			}

			// Planets
			dot.setPointCount(30);
			dot.setRotation(0.0f);

			for (Planet& planet : currentStar->getPlanets()) {
				if (Math::distance(planet.getPos(), currentStar->getLocalViewCenter()) < m_minimapCircle.getRadius()) {
					dot.setPosition(planet.getPos());

					switch (planet.getType()) {
					case Planet::PLANET_TYPE::TERRA:
						dot.setFillColor(sf::Color(0, 255, 0));
						break;
					case Planet::PLANET_TYPE::BARREN:
						dot.setFillColor(sf::Color(125, 125, 125));
						break;
					case Planet::PLANET_TYPE::DESERT:
						dot.setFillColor(sf::Color(200, 150, 100));
						break;
					case Planet::PLANET_TYPE::GAS_GIANT:
						dot.setFillColor(sf::Color(255, 125, 125));
						break;
					case Planet::PLANET_TYPE::ICE_GIANT:
						dot.setFillColor(sf::Color::Blue);
						break;
					case Planet::PLANET_TYPE::LAVA:
					case Planet::PLANET_TYPE::VOLCANIC:
						dot.setFillColor(sf::Color(255, 200, 200));
						break;
					case Planet::PLANET_TYPE::OCEAN:
						dot.setFillColor(sf::Color(125, 125, 255));
						break;
					case Planet::PLANET_TYPE::TOXIC:
						dot.setFillColor(sf::Color::Yellow);
						break;
					case Planet::PLANET_TYPE::TUNDRA:
						dot.setFillColor(sf::Color::White);
						break;
					default:
						dot.setFillColor(sf::Color(255, 0, 255));
					}
					window.draw(dot);
				}
			}
		}
		
		// Jump points
		dot.setFillColor(sf::Color(128, 0, 128));

		for (auto& jumpPoint : currentStar->getJumpPoints()) {
			dot.setPosition(jumpPoint.getPos());
			window.draw(dot);
		}

		window.setView(oldView);
	}
}

void MinimapGUI::update(const sf::RenderWindow& window, GameState& state, const UnitGUI& unitGUI) {
	if (state.getState() == GameState::State::LOCAL_VIEW && !unitGUI.isSelecting()) {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
			sf::Vector2i pos = sf::Mouse::getPosition(window);
			sf::Vector2u windowSize = window.getSize();
			sf::Vector2f normalPos = sf::Vector2f(static_cast<float>(pos.x) / windowSize.x, static_cast<float>(pos.y) / windowSize.y);
			sf::FloatRect viewport = m_view.getViewport();
			sf::Vector2f relPos = sf::Vector2f(normalPos.x - viewport.left, normalPos.y - viewport.top);
			sf::Vector2f worldPos = sf::Vector2f((relPos.x - viewport.width / 2.0f) * m_view.getSize().x * (1.0f / viewport.width) + m_view.getCenter().x, (relPos.y - viewport.height / 2.0f) * m_view.getSize().y * (1.0f / viewport.height) + m_view.getCenter().y);

			if (Math::distance(m_view.getCenter(), worldPos) < m_minimapCircle.getRadius()) {
				state.getCamera().setPos(worldPos);
			}
		}
	}
}

bool MinimapGUI::isMouseInMinimap(const sf::RenderWindow& window) {
	sf::Vector2i mousePos = sf::Mouse::getPosition(window);
	return m_view.getViewport().contains(sf::Vector2f(static_cast<float>(mousePos.x) / window.getSize().x, static_cast<float>(mousePos.y) / window.getSize().y));
}

void MinimapGUI::onEvent(const sf::Event& ev) {
	if (ev.type == sf::Event::KeyPressed) {
		if (Keybindings::isKeyPress("HideUI", ev)) {
			m_hidden = !m_hidden;
		}
	}
}