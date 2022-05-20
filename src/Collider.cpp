#include "gamepch.h"
#include "Collider.h"
#include "Math.h"
#include "Renderer.h"

Collider::Collider() {
	setOutlineThickness(5.0f);
	setFillColor(sf::Color::Transparent);
}

Collider::Collider(const sf::Vector2f& pos, const sf::Color& color, float radius) {
	setFillColor(sf::Color::Transparent);
	setOutlineColor(color);
	setOutlineThickness(5.0f);
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

bool Collider::isCollidingWith(const Collider& other) const {
	float totalRadius = getRadius() + other.getRadius();
	if (Math::distance(getPosition(), other.getPosition()) < totalRadius) return true;
	else return false;
}

bool Collider::contains(const sf::Vector2f& point) const {
	if (Math::distance(point, getPosition()) < getRadius()) return true;
	return false;
}

void Collider::draw(Renderer& renderer, sf::Vector2f mouseWorldPos) {
	sf::Color prevColor = getColor();
	if (contains(mouseWorldPos)) {
		setColor(prevColor + sf::Color(125, 125, 125));
	}
	renderer.draw(*this);
	setColor(prevColor);
}