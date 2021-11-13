#include "gamepch.h"
#include "MinimapGUI.h"
#include "../Star.h"
#include "../Math.h"
#include "UnitGUI.h"

void MinimapGUI::draw(sf::RenderWindow& window, Star* currentStar, int playerAllegiance, Camera& camera) {
	if (currentStar != nullptr) {
		sf::View oldView = window.getView();
		float ratio = oldView.getSize().x / oldView.getSize().y; // 1.7786

		m_view.setCenter(currentStar->getPos());
		m_view.setSize(50000.0f, 50000.0f);
		m_view.setViewport(sf::FloatRect(1.0f - 0.4f / ratio, 0.6f, 0.4f / ratio, 0.4f));

		window.setView(m_view);

		m_minimapCircle.setFillColor(sf::Color(125, 125, 125, 125));
		m_minimapCircle.setPosition(currentStar->getPos());
		m_minimapCircle.setRadius(25000.0f);
		m_minimapCircle.setOrigin(25000.0f, 25000.0f);
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
		dot.setRadius(500.0f);
		dot.setOrigin(500.0f, 500.0f);

		window.draw(dot);

		if ((currentStar->isDrawingHidden() && currentStar->isDiscovered(playerAllegiance)) || playerAllegiance == -1) {
			for (auto& ship : currentStar->getSpaceships()) {
				if (Math::distance(ship->getPos(), currentStar->getLocalViewCenter()) < m_minimapCircle.getRadius()) {
					dot.setPosition(ship->getPos());
					if (playerAllegiance == -1) {
						dot.setFillColor(ship->getFactionColor());
					}
					else {
						if (ship->getAllegiance() == playerAllegiance) {
							dot.setFillColor(sf::Color::Blue);
						}
						else {
							dot.setFillColor(sf::Color::Red);
						}
					}

					window.draw(dot);
				}
			}
		}

		for (auto& jumpPoint : currentStar->getJumpPoints()) {
			dot.setPosition(jumpPoint.getPos());
			dot.setFillColor(sf::Color(128, 0, 128));
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