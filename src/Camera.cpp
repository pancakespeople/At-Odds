#include "gamepch.h"

#include "Camera.h"
#include "Math.h"
#include "Renderer.h"
#include "Keybindings.h"

#include <iostream>

Camera::Camera(float x, float y, float w, float h) {
	m_view = sf::View(sf::FloatRect(x, y, w, h));
	m_initialRect = sf::FloatRect(x, y, w, h);
	m_camZoomFactor = 1;
}

Camera::Camera() {
	m_camZoomFactor = 1;
}

void Camera::update(Renderer& renderer, tgui::Widget::Ptr focusedWidget) {
	tgui::String widgetType;
	if (focusedWidget != nullptr) {
		widgetType = focusedWidget->getWidgetType();
	}
	
	if (m_allowMovement) {
		// Move the camera by clicking and dragging
		static sf::Vector2i lastpos = sf::Mouse::getPosition();

		if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
			sf::Vector2i nextpos = lastpos - sf::Mouse::getPosition();
			m_view.move(sf::Vector2f(nextpos.x * m_camZoomFactor, nextpos.y * m_camZoomFactor));
		}

		lastpos = sf::Mouse::getPosition();

		m_view.move(m_velocity * m_camZoomFactor);
	}

	// Smooth zooming
	m_camZoomFactor = Math::lerp(m_camZoomFactor, m_wantedZoomFactor, Math::clamp(m_zoomTimer.getElapsedTime().asSeconds(), 0.0f, 1.0f));
	m_view.setSize(sf::Vector2f(m_initialRect.width, m_initialRect.height) * m_camZoomFactor);

	renderer.setView(m_view);
}

void Camera::onEvent(sf::Event& ev) {
	// Zoom with the mouse wheel
	if (ev.type == sf::Event::MouseWheelScrolled) {
		if (ev.mouseWheelScroll.delta >= 1) {
			//m_view.zoom(1 - ZOOM_FACTOR);
			m_wantedZoomFactor *= 1 - zoomChange;
			m_zoomTimer.restart();
		}
		else {
			//m_view.zoom(1 + ZOOM_FACTOR);
			m_wantedZoomFactor *= 1 + zoomChange;
			m_zoomTimer.restart();
		}
	}
	else if (ev.type == sf::Event::KeyPressed) {
		if (Keybindings::isKeyPress("CameraLeft", ev) || Keybindings::isKeyPress("AltCameraLeft", ev)) {
			m_velocity.x = -10.0f;
		}
		if (Keybindings::isKeyPress("CameraUp", ev) || Keybindings::isKeyPress("AltCameraUp", ev)) {
			m_velocity.y = -10.0f;
		}
		if (Keybindings::isKeyPress("CameraDown", ev) || Keybindings::isKeyPress("AltCameraDown", ev)) {
			m_velocity.y = 10.0f;
		}
		if (Keybindings::isKeyPress("CameraRight", ev) || Keybindings::isKeyPress("AltCameraRight", ev)) {
			m_velocity.x = 10.0f;
		}
	}
	else if (ev.type == sf::Event::KeyReleased) {
		if (Keybindings::isKeyRelease("CameraLeft", ev) || Keybindings::isKeyRelease("AltCameraLeft", ev)) {
			m_velocity.x = 0.0f;
		}
		if (Keybindings::isKeyRelease("CameraUp", ev) || Keybindings::isKeyRelease("AltCameraUp", ev)) {
			m_velocity.y = 0.0f;
		}
		if (Keybindings::isKeyRelease("CameraDown", ev) || Keybindings::isKeyRelease("AltCameraDown", ev)) {
			m_velocity.y = 0.0f;
		}
		if (Keybindings::isKeyRelease("CameraRight", ev) || Keybindings::isKeyRelease("AltCameraRight", ev)) {
			m_velocity.x = 0.0f;
		}
	}
}

void Camera::zoom(float factor) {
	m_view.zoom(factor);
	m_wantedZoomFactor *= factor;
	m_camZoomFactor *= factor;
}

void Camera::resetZoom() {
	m_view.setSize(getInitialWidthHeight());
	m_camZoomFactor = 1.0f;
	m_wantedZoomFactor = 1.0f;
}

void Camera::setAbsoluteZoom(float factor) {
	resetZoom();
	zoom(factor);
}

void Camera::setScreenSize(sf::Vector2f size) {
	m_view.setSize(size * m_camZoomFactor);
}

sf::FloatRect Camera::getRect() const {
	sf::FloatRect rect;
	rect.left = m_view.getCenter().x - m_view.getSize().x / 2.0f;
	rect.top = m_view.getCenter().y - m_view.getSize().y / 2.0f;
	rect.width = m_view.getSize().x;
	rect.height = m_view.getSize().y;
	return rect;
}