#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

class DebugInfo {
public:
	DebugInfo(const sf::RenderWindow& window);
	void draw(sf::RenderWindow& window, float fps, int ticks);

private:
	std::vector<float> m_fpsTimes;
	sf::Text m_fpsText;
	float m_realFps = 0.0f;
};