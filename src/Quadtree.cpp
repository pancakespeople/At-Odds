#include "gamepch.h"
#include "Quadtree.h"
#include "Unit.h"
#include "Renderer.h"
#include "Fonts.h"

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

	m_subNodes[0].m_level = m_level + 1;
	m_subNodes[1].m_level = m_level + 1;
	m_subNodes[2].m_level = m_level + 1;
	m_subNodes[3].m_level = m_level + 1;

	m_subNodes[0].m_quadrant = 0;
	m_subNodes[1].m_quadrant = 1;
	m_subNodes[2].m_quadrant = 2;
	m_subNodes[3].m_quadrant = 3;

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
	
	// Check outer bounds
	if (colliderPos.x - collider.getRadius() >= m_middle.x - m_size / 2.0f && colliderPos.x + collider.getRadius() <= m_middle.x + m_size / 2.0f &&
		colliderPos.y - collider.getRadius() >= m_middle.y - m_size / 2.0f && colliderPos.y + collider.getRadius() <= m_middle.y + m_size / 2.0f) {
		// Check inner bounds

		if (colliderPos.y + collider.getRadius() <= m_middle.y) {
			// Top quadrants

			if (colliderPos.x - collider.getRadius() >= m_middle.x) {
				return 0; // Top right
			}
			if (colliderPos.x + collider.getRadius() <= m_middle.x) {
				return 1; // Top left
			}

			return -1;
		}
		if (colliderPos.y - collider.getRadius() >= m_middle.y) {
			// Bottom quadrants

			if (colliderPos.x - collider.getRadius() >= m_middle.x) {
				return 3; // Bottom right
			}
			if (colliderPos.x + collider.getRadius() <= m_middle.x) {
				return 2; // Bottom left
			}

			return -1;
		}

		return -1;
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

void Quadtree::draw(Renderer& renderer) {
	sf::RectangleShape rect;
	rect.setPosition(m_middle);
	rect.setSize(sf::Vector2f(m_size, m_size));
	rect.setOrigin(sf::Vector2f(m_size / 2.0f, m_size / 2.0f));
	rect.setFillColor(sf::Color::Transparent);
	rect.setOutlineThickness(25.0f);
	rect.setOutlineColor(sf::Color::Red);

	renderer.draw(rect);

	sf::Text text;
	text.setFont(Fonts::getFont(Fonts::MAIN_FONT_PATH));
	text.setOrigin({ text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f });

	for (Unit* unit : m_objects) {
		text.setString(std::to_string(m_level) + " : " + std::to_string(m_quadrant));
		text.setScale({ 5.0, 5.0 });
		text.setPosition(unit->getPos());
		renderer.draw(text);
	}

	for (Quadtree& subNode : m_subNodes) {
		subNode.draw(renderer);
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
	outObjects.insert(outObjects.end(), m_objects.begin(), m_objects.end());
}