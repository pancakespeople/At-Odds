#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

class Animation {
public:
	enum class ANIMATION_TYPE {
		EXPLOSION
	};
	
	Animation(const std::string& type, sf::Vector2f pos);

	Animation(const std::string& filePath, int frameWidth, int frameHeight, sf::Vector2f pos, int frameDelay = 0, float scale = 1.0f);

	void draw(sf::RenderWindow& window);

	bool isDone() { return m_done; }

	void nextFrame();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive& m_frames;
		archive& m_frameIndex;
		archive& m_frameDelay;
		archive& m_ticksToNextFrame;
		archive& m_done;
	}

	Animation() {}
	
	void init(const std::string& filePath, int frameWidth, int frameHeight, sf::Vector2f pos, int frameDelay = 0, float scale = 1.0f);

	std::vector<sf::Sprite> m_frames;
	int m_frameIndex = 0;
	int m_frameDelay = 0;
	int m_ticksToNextFrame = 0;
	bool m_done = false;
};

