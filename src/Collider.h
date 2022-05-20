#pragma once
#include <SFML/Graphics.hpp>
#include "SmoothCircle.h"

class Renderer;

class Collider : public sf::CircleShape {
public:
	Collider();
	
	Collider(const sf::Vector2f& pos, const sf::Color& color, float radius);

	void setRadius(float radius);
	void setColor(sf::Color color) { sf::CircleShape::setOutlineColor(color); }
	void update(const sf::Vector2f& pos);
	void draw(Renderer& renderer, sf::Vector2f mouseWorldPos);

	bool isCollidingWith(const Collider& other) const;
	bool contains(const sf::Vector2f& point) const;

	sf::Color getColor() const { return sf::CircleShape::getOutlineColor(); }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& boost::serialization::base_object<sf::CircleShape> (*this);
	}
};

