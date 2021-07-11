#include "gamepch.h"

#include "Camera.h"

#include <iostream>

Camera::Camera(float x, float y, float w, float h) {
	m_view = sf::View(sf::FloatRect(x, y, w, h));
	m_initialRect = sf::FloatRect(x, y, w, h);
	m_camZoomFactor = 1;
}

Camera::Camera() {
	m_camZoomFactor = 1;
}

void Camera::update(sf::RenderWindow& window, tgui::Widget::Ptr focusedWidget) {
	tgui::String widgetType;
	if (focusedWidget != nullptr) {
		widgetType = focusedWidget->getWidgetType();
	}
	
	// Move the camera by clicking and dragging
	static sf::Vector2i lastpos = sf::Mouse::getPosition();

	if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
		sf::Vector2i nextpos = lastpos - sf::Mouse::getPosition();
		m_view.move(sf::Vector2f(nextpos.x * m_camZoomFactor, nextpos.y * m_camZoomFactor));
	}

	lastpos = sf::Mouse::getPosition();

	// Use WASD or arrow keys to move camera
	if (widgetType != "EditBox") {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			m_view.move(sf::Vector2f(-10.0f * m_camZoomFactor, 0.0f));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			m_view.move(sf::Vector2f(0.0f, -10.0f * m_camZoomFactor));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			m_view.move(sf::Vector2f(0.0f, 10.0f * m_camZoomFactor));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			m_view.move(sf::Vector2f(10.0f * m_camZoomFactor, 0.0f));
		}
	}

	window.setView(m_view);
}

void Camera::zoomEvent(sf::Event& ev) {
	// Zoom with the mouse wheel
	if (ev.type == sf::Event::MouseWheelScrolled) {
		if (ev.mouseWheelScroll.delta >= 1) {
			m_view.zoom(1 - ZOOM_FACTOR);
			m_camZoomFactor *= 1 - ZOOM_FACTOR;
		}
		else {
			m_view.zoom(1 + ZOOM_FACTOR);
			m_camZoomFactor *= 1 + ZOOM_FACTOR;
		}
	}
}

void Camera::zoom(float factor) {
	m_view.zoom(factor);
	m_camZoomFactor *= factor;
}

void Camera::resetZoom() {
	m_view.setSize(getInitialWidthHeight());
	m_camZoomFactor = 1;
}

void Camera::setAbsoluteZoom(float factor) {
	resetZoom();
	zoom(factor);
}