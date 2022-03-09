#pragma once
#include <array>
#include <SFML/Graphics.hpp>

class Collider;
class Unit;
class Renderer;

class Quadtree {
public:
	static const int maxObjects = 10;
	static const int maxLevels = 5;
	
	Quadtree(sf::Vector2f middle, float size);
	Quadtree() {}

	void insert(Unit* object);
	void split();
	int getQuadrant(const Collider& collider);
	void draw(Renderer& window);
	void clear();
	void retrieve(std::vector<Unit*>& outObjects, const Collider& object);

private:
	sf::Vector2f m_middle;
	float m_size = 0.0f;
	std::vector<Unit*> m_objects;
	std::vector<Quadtree> m_subNodes;
	int m_level = 0;
	int m_quadrant = -1;
};