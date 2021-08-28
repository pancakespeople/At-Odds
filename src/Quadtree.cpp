#include "gamepch.h"
#include "Quadtree.h"
#include "Unit.h"

Quadtree::Quadtree(sf::Vector2f middle, float size) {
	m_middle = middle;
	m_size = size;
}

void Quadtree::split() {
	float subSize = m_size / 2.0f;
	float middleStep = m_size / 4.0f;

	m_subNodes.push_back(Quadtree(sf::Vector2f(m_middle.x + middleStep, m_middle.y + middleStep), subSize)); // Top right
	m_subNodes.push_back(Quadtree(sf::Vector2f(m_middle.x - middleStep, m_middle.y + middleStep), subSize)); // Top left
	m_subNodes.push_back(Quadtree(sf::Vector2f(m_middle.x - middleStep, m_middle.y - middleStep), subSize)); // Bottom left
	m_subNodes.push_back(Quadtree(sf::Vector2f(m_middle.x + middleStep, m_middle.y - middleStep), subSize)); // Bottom right

	// Put objects into subnodes
	for (int i = 0; i < m_objects.size(); i++) {
		int quadrant = getQuadrant(m_objects[i]->getCollider());
		if (quadrant != -1) {
			m_subNodes[quadrant].insert(m_objects[i]);
			m_objects.erase(m_objects.begin() + i);
			i--;
		}
	}
}

int Quadtree::getQuadrant(const Collider& collider) {
	sf::Vector2f colliderPos = collider.getPosition();

	// Check bounds
	if (colliderPos.x - collider.getRadius() >= m_middle.x - m_size / 2.0f && colliderPos.x + collider.getRadius() <= m_middle.x + m_size / 2.0f &&
		colliderPos.y - collider.getRadius() >= m_middle.y - m_size / 2.0f && colliderPos.y + collider.getRadius() <= m_middle.y + m_size / 2.0f) {
		if (colliderPos.y >= m_middle.y) {
			// Top quadrants

			if (colliderPos.x >= m_middle.x) {
				return 0; // Top right
			}
			else {
				return 1; // Top left
			}
		}
		else {
			// Bottom quadrants

			if (colliderPos.x >= m_middle.x) {
				return 3; // Bottom right
			}
			else {
				return 2; // Bottom left
			}
		}
	}
	else {
		// Doesn't fit
		return -1;
	}
}

void Quadtree::insert(Unit* object) {
	int quadrant = getQuadrant(object->getCollider());

	if (m_subNodes.size() > 0 && quadrant != -1) {
		m_subNodes[quadrant].insert(object);
	}
	else {
		if (m_objects.size() >= maxObjects && m_subNodes.size() == 0 && quadrant != -1) {
			m_objects.push_back(object);
			split();
		}
		else {
			m_objects.push_back(object);
		}
	}
}

void Quadtree::draw(sf::RenderWindow& window) {
	sf::RectangleShape rect;
	rect.setPosition(m_middle);
	rect.setSize(sf::Vector2f(m_size, m_size));
	rect.setOrigin(sf::Vector2f(m_size / 2.0f, m_size / 2.0f));
	rect.setFillColor(sf::Color::Transparent);
	rect.setOutlineThickness(25.0f);
	rect.setOutlineColor(sf::Color::Red);

	window.draw(rect);

	for (Quadtree& subNode : m_subNodes) {
		subNode.draw(window);
	}
}

void Quadtree::clear() {
	m_objects.clear();
	for (Quadtree& subNode : m_subNodes) {
		subNode.clear();
	}
	m_subNodes.clear();
}

void Quadtree::retrieve(std::vector<Unit*>& outObjects, const Collider& object) {
	int quadrant = getQuadrant(object);
	if (quadrant != -1 && m_subNodes.size() > 0) {
		m_subNodes[quadrant].retrieve(outObjects, object);
	}
	else {
		outObjects = m_objects;
	}
}