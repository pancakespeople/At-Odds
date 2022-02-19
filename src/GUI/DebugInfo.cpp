#include "gamepch.h"
#include "DebugInfo.h"
#include "../Fonts.h"

DebugInfo::DebugInfo(const sf::RenderWindow& window) {
    m_text.setFont(Fonts::getFont("data/fonts/OpenSans-Regular.ttf"));
    m_text.setPosition(sf::Vector2f(0.0f, window.getSize().y - m_text.getCharacterSize()));
}

void DebugInfo::draw(sf::RenderWindow& window) {
    const sf::View& oldView = window.getView();
    window.setView(window.getDefaultView());
    window.draw(m_text);
    window.setView(oldView);
}

void DebugInfo::update(float fps, int frameTime, int ticks, int upf) {
    m_fpsTimes.push_back(fps);
    if (ticks % 60 == 0) {
        m_realFps = 0;
        for (int i = 0; i < m_fpsTimes.size(); i++) {
            m_realFps += m_fpsTimes[i];
        }
        m_realFps /= m_fpsTimes.size();
        m_fpsTimes.clear();
    }

    m_text.setString("FPS: " + std::to_string(m_realFps) + " Frametime: " + std::to_string(frameTime) + " Updates: " + std::to_string(upf));
}