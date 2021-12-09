#include "gamepch.h"
#include "DebugInfo.h"
#include "../Fonts.h"

DebugInfo::DebugInfo(const sf::RenderWindow& window) {
	m_fpsText.setFont(Fonts::getFont("data/fonts/consola.ttf"));
	m_fpsText.setPosition(sf::Vector2f(0.0f, window.getSize().y - m_fpsText.getCharacterSize()));
}

void DebugInfo::draw(sf::RenderWindow& window, float fps, int ticks) {
    m_fpsTimes.push_back(fps);
    if (ticks % 60 == 0) {
        m_realFps = 0;
        for (int i = 0; i < m_fpsTimes.size(); i++) {
            m_realFps += m_fpsTimes[i];
        }
        m_realFps /= m_fpsTimes.size();
        m_fpsTimes.clear();
    }

    m_fpsText.setString(std::to_string(m_realFps));

    const sf::View& oldView = window.getView();
    window.setView(window.getDefaultView());
    window.draw(m_fpsText);
    window.setView(oldView);
}