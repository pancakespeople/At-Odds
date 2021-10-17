#pragma once
#include <SFML/Graphics.hpp>
#include "SmoothCircle.h"

class Collider : public SmoothCircle {
public:
	Collider();
	
	Collider(const sf::Vector2f& pos, const sf::Color& color, float radius);

	void setRadius(float radius);
	void update(const sf::Vector2f& pos);

	bool isCollidingWith(const Collider& other) const;
	bool contains(const sf::Vector2f& point) const;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& boost::serialization::base_object<SmoothCircle>(*this);
	}
};

