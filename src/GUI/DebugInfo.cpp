#include "gamepch.h"
#include "DebugInfo.h"
#include "../Fonts.h"

DebugInfo::DebugInfo(const sf::RenderWindow& window) {
    m_text.setFont(Fonts::getFont("data/fonts/consola.ttf"));
    m_text.setPosition(sf::Vector2f(0.0f, window.getSize().y - m_text.getCharacterSize()));
}

void DebugInfo::draw(sf::RenderWindow& window) {
    const sf::View& oldView = window.getView();
    window.setView(window.getDefaultView());
    window.draw(m_text);
    window.setView(oldView);
}

void DebugInfo::update(float fps, int frameTime, int ticks, float updateStep, float updateTarget) {
    m_fpsTimes.push_back(fps);
    m_upsTimes.push_back(std::round(updateStep) * updateTarget);
    if (ticks % 60 == 0) {
        m_realFps = 0;
        for (int i = 0; i < m_fpsTimes.size(); i++) {
            m_realFps += m_fpsTimes[i];
        }
        m_realFps /= m_fpsTimes.size();
        m_fpsTimes.clear();

        m_realUps = 0;
        for (int i = 0; i < m_upsTimes.size(); i++) {
            m_realUps += m_upsTimes[i];
        }
        m_realUps /= m_upsTimes.size();
        m_upsTimes.clear();
    }

    m_text.setString("FPS: " + std::to_string(m_realFps) + " Frametime: " + std::to_string(frameTime) + " UPS: " + std::to_string(m_realUps));
}