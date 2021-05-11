#pragma once
#include <SFML/Graphics.hpp>

#define ZOOM_FACTOR 0.25f

class Camera {
public:
	Camera(float x, float y, float w, float h);
	
	Camera();

	void update(sf::RenderWindow& window);

	void zoomEvent(sf::Event ev);

	void setPos(sf::Vector2f pos) { m_view.setCenter(pos); }

	void zoom(float factor);

	void setAbsoluteZoom(float factor);

	void resetZoom();

	sf::FloatRect getInitialRect() { return m_initialRect; }

	sf::Vector2f getInitialWidthHeight() { return sf::Vector2f(m_initialRect.width, m_initialRect.height); }
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& m_view;
		archive& m_camZoomFactor;
		archive& m_initialRect;
	}
	
	sf::View m_view;
	float m_camZoomFactor = 1;
	sf::FloatRect m_initialRect;
};

