#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

class Animation {
public:
	Animation(const std::string& filePath, int frameWidth, int frameHeight, sf::Vector2f pos, int frameDelay = 0, float scale = 1.0f);

	void draw(sf::RenderWindow& window);

	bool isDone() { return m_done; }

	void nextFrame();

private:
	
	std::vector<sf::Sprite> m_frames;
	int m_frameIndex = 0;
	int m_frameDelay;
	int m_ticksToNextFrame = 0;
	bool m_done = false;
};

