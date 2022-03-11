#pragma once
#include <boost/serialization/access.hpp>

class Star;
class Renderer;

class AsteroidBelt {
public:
	AsteroidBelt(uint64_t seed, float radius) : m_seed(seed), m_radius(radius) {}
	
	void generate(const Star& star);
	void draw(Renderer& renderer, const Star& star);
	void clearVertices() { m_vertices.clear(); m_generated = false; }

	bool isGenerated() { return m_generated; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_seed;
		archive & m_radius;
	}
	
	AsteroidBelt() = default;

	uint64_t m_seed = 0;
	float m_radius = 0.0f;
	bool m_generated = false;

	std::vector<sf::Vertex> m_vertices;
};