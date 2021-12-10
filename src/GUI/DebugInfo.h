#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

class DebugInfo {
public:
	DebugInfo(const sf::RenderWindow& window);
	void draw(sf::RenderWindow& window);
	void update(float fps, int frameTime, int ticks, float updateStep, float updateTarget);

private:
	std::vector<float> m_fpsTimes;
	std::vector<float> m_upsTimes;

	sf::Text m_text;
	float m_realFps = 0.0f;
	float m_realUps = 0.0f;
};