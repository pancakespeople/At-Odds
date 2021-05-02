#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

class Animation {
public:
	enum class ANIMATION_TYPE {
		EXPLOSION
	};
	
	Animation(ANIMATION_TYPE type, sf::Vector2f pos);

	Animation(const std::string& filePath, int frameWidth, int frameHeight, sf::Vector2f pos, int frameDelay = 0, float scale = 1.0f);

	void draw(sf::RenderWindow& window);

	bool isDone() { return m_done; }

	void nextFrame();

private:
	void init(const std::string& filePath, int frameWidth, int frameHeight, sf::Vector2f pos, int frameDelay = 0, float scale = 1.0f);

	std::vector<sf::Sprite> m_frames;
	int m_frameIndex = 0;
	int m_frameDelay;
	int m_ticksToNextFrame = 0;
	bool m_done = false;
};

