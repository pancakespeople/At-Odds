#pragma once
#include <SFML/Graphics.hpp>

namespace Math {

	const double pi = 3.14159265358979323846;

	const double toRadians = pi / 180.0;

	const double toDegrees = 180.0 / pi;

	inline double distance(const sf::Vector2f& a, const sf::Vector2f& b) {
		return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2));
	}

	// Returns degrees
	inline double angleBetween(const sf::Vector2f& a, const sf::Vector2f& b) {
		sf::Vector2f dist;

		dist.x = b.x - a.x;
		dist.y = b.y - a.y;

		float angleDegrees = std::atan(-dist.y / dist.x) * Math::toDegrees;
		angleDegrees += 180.0;

		if (a.x < b.x) {
			angleDegrees = angleDegrees - 180.0;
		}

		if (angleDegrees < 0.0) {
			angleDegrees += 360.0;
		}

		return angleDegrees;
	}
}