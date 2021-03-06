#pragma once
#include <SFML/Graphics.hpp>

class Renderer;

class Camera {
public:
	Camera(float x, float y, float w, float h);
	Camera();
	
	void update(Renderer& renderer);
	void onEvent(sf::Event& ev, tgui::Widget::Ptr focusedWidget);
	void setPos(sf::Vector2f pos) { m_view.setCenter(pos); }
	void zoom(float factor);
	void setAbsoluteZoom(float factor);
	void resetZoom();
	void setScreenSize(sf::Vector2f size);
	void setAllowMovement(bool allow) { m_allowMovement = allow; }

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
	sf::Vector2f m_velocity;

	float m_camZoomFactor = 1.0f;
	float m_wantedZoomFactor = 1.0f;
	const inline static float zoomChange = 0.5f;

	bool m_allowMovement = true;
};

