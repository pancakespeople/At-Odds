#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

class DebugInfo {
public:
	DebugInfo(const sf::RenderWindow& window);
	void draw(sf::RenderWindow& window);
	void update(float fps, int frameTime, int ticks, int upf);

private:
	std::vector<float> m_fpsTimes;

	sf::Text m_text;
	float m_realFps = 0.0f;
};