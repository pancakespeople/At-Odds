#pragma once

class Orbit {
public:
	Orbit() {}
	
	Orbit(sf::Vector2f orbiterPos, sf::Vector2f parentPos);

	sf::Vector2f update();

	void draw(sf::RenderWindow& window);
	void setParentPos(sf::Vector2f pos) { m_parentPos = pos; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_radius;
		archive & m_orbiterAngle;
		archive & m_orbitSpeed;
		archive & m_orbitIndicator;
		archive & m_parentPos;
	}
	
	float m_radius = 0.0f;
	float m_orbiterAngle = 0.0f;
	float m_orbitSpeed = 0.01f;
	
	sf::CircleShape m_orbitIndicator;
	sf::Vector2f m_parentPos;
};