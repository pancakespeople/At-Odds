#pragma once
#include <SFML/Graphics.hpp>

class Collider : public sf::CircleShape {
public:
	Collider();
	
	Collider(const sf::Vector2f& pos, const sf::Color& color, float radius);

	void setRadius(float radius);

	void update(const sf::Vector2f& pos);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& boost::serialization::base_object<sf::CircleShape>(*this);
	}
};

