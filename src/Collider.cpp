#include "gamepch.h"
#include "Collider.h"
#include "Math.h"

Collider::Collider() {
	setOutlineThickness(5.0f);
	setBlur(0.05);
}

Collider::Collider(const sf::Vector2f& pos, const sf::Color& color, float radius) {
	setColor(color);
	setOutlineThickness(5.0f);
	setBlur(0.05);
	setRadius(radius);
	setPosition(pos);
}

void Collider::update(const sf::Vector2f& pos) {
	setPosition(pos);
}

void Collider::setRadius(float radius) {
	setOrigin(sf::Vector2f(radius, radius));
	SmoothCircle::setRadius(radius);
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