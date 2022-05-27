#include "gamepch.h"
#include "BuildingGUI.h"
#include "../Math.h"
#include "../Constellation.h"
#include "../Renderer.h"

void BuildingGUI::onEvent(const sf::Event& ev, const sf::RenderWindow& window, Renderer& renderer, tgui::Gui& gui, GameState& state, Constellation& constellation, tgui::Panel::Ptr mainPanel) {
	bool valid = ev.mouseButton.button == sf::Mouse::Left && state.getLocalViewStar() != nullptr && mainPanel->isFocused();

	if (ev.type == sf::Event::EventType::MouseButtonReleased && valid) {
		sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
		sf::Vector2f mouseWorldPos = renderer.mapPixelToCoords(mouseScreenPos);

		if (m_window != nullptr) {
			if (m_window->isMouseOnWidget(tgui::Vector2f(mouseScreenPos.x, mouseScreenPos.y))) {
				return;
			}
		}

		for (auto& building : state.getLocalViewStar()->getBuildings()) {
			float dist = Math::distance(building->getPos(), mouseWorldPos);
			float downDist = Math::distance(building->getPos(), m_lastMouseDownPos);
			if (dist < building->getCollider().getRadius() && state.getPlayer().getFaction() == building->getAllegiance() &&
				downDist < building->getCollider().getRadius()) {
				if (m_window != nullptr) {
					gui.remove(m_window);
				}

				m_window = GUI::ChildWindow::create();
				m_window->getRenderer()->setOpacity(0.75f);
				m_window->setSize("10%", "15%");
				m_window->setOrigin(0.5f, 0.5f);
				m_window->setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
				m_window->setTitle(building->getName());
				m_window->onClose([this]() {
					m_window = nullptr;
					m_openBuilding = nullptr;
					});
				gui.add(m_window);

				m_openBuilding = building.get();
				building->openModGUI(m_window, constellation.getFaction(state.getPlayer().getFaction()));

				break;
			}
		}
	}
	else if (ev.type == sf::Event::EventType::MouseButtonPressed && valid) {
		m_lastMouseDownPos = renderer.mapPixelToCoords(sf::Mouse::getPosition(window));
	}
}

void BuildingGUI::update() {
	// Close the window on death of the building so the game doesn't crash
	if (m_openBuilding != nullptr) {
		if (m_openBuilding->isDead()) {
			m_window->close();
		}
	}
}