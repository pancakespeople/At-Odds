#pragma once
#include <SFML/Graphics.hpp>

class Renderer;

class Camera {
public:
	Camera(float x, float y, float w, float h);
	Camera();
	
	void update(Renderer& renderer, tgui::Widget::Ptr focusedWidget);
	void zoomEvent(sf::Event& ev);
	void setPos(sf::Vector2f pos) { m_view.setCenter(pos); }
	void zoom(float factor);
	void setAbsoluteZoom(float factor);
	void resetZoom();
	void setScreenSize(sf::Vector2f size);

	sf::FloatRect getInitialRect() const { return m_initialRect; }
	sf::Vector2f getInitialWidthHeight() const { return sf::Vector2f(m_initialRect.width, m_initialRect.height); }
	sf::Vector2f getPos() const { return m_view.getCenter(); }
	sf::FloatRect getRect() const;
	const sf::View getView() const { return m_view; }

	float getZoomFactor() const { return m_camZoomFactor; }

private:
	friend class boost::serialization::access;
	BOOST_SERIALIZATION_SPLIT_MEMBER()
	template<class Archive>
	void save(Archive& archive, const unsigned int version) const {
		archive & m_view;
		archive & m_camZoomFactor;
		archive & m_initialRect;	
	}

	template<class Archive>
	void load(Archive& archive, const unsigned int version) {
		archive & m_view;
		archive & m_camZoomFactor;
		archive & m_initialRect;

		m_wantedZoomFactor = m_camZoomFactor;
	}
	
	sf::View m_view;
	sf::FloatRect m_initialRect;
	sf::Clock m_zoomTimer;

	float m_camZoomFactor = 1.0f;
	float m_wantedZoomFactor = 1.0f;
	const inline static float zoomChange = 0.5f;
};

