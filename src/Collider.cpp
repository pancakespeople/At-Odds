#include "gamepch.h"
#include "Collider.h"

Collider::Collider() {
	setOutlineThickness(10.0f);
	setFillColor(sf::Color::Transparent);
}

Collider::Collider(const sf::Vector2f& pos, const sf::Color& color, float radius) {
	setOutlineColor(color);
	setOutlineThickness(10.0f);
	setFillColor(sf::Color::Transparent);
	setRadius(radius);
	setPosition(pos);
}

void Collider::update(const sf::Vector2f& pos) {
	setPosition(pos);
}

void Collider::setRadius(float radius) {
	setOrigin(sf::Vector2f(radius, radius));
	sf::CircleShape::setRadius(radius);
}
