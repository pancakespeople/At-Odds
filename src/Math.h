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

	inline double magnitude(const sf::Vector2f& vec) {
		return std::sqrt(vec.x * vec.x + vec.y * vec.y);
	}

	inline double lerp(double v0, double v1, double t) {
		return (1 - t) * v0 + t * v1;
	}

	inline sf::Vector2f lerp(const sf::Vector2f& vec1, const sf::Vector2f vec2, double t) {
		return sf::Vector2f(lerp(vec1.x, vec2.x, t), lerp(vec1.y, vec2.y, t));
	}

	inline double clamp(double val, double min, double max) {
		return std::clamp(val, min, max);
	}

	inline sf::Color mix(const sf::Color& color1, const sf::Color& color2, double t) {
		return sf::Color(lerp(color1.r, color2.r, t), lerp(color1.g, color2.g, t), lerp(color1.b, color2.b, t), lerp(color1.a, color2.a, t));
	}

	inline sf::Vector2f normalize(const sf::Vector2f vec) {
		float mag = magnitude(vec);
		return vec / mag;
	}
}