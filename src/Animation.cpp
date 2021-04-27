#include "gamepch.h"

#include "Animation.h"
#include "TextureCache.h"

Animation::Animation(const std::string& filePath, int frameWidth, int frameHeight, sf::Vector2f pos, int frameDelay, float scale) {
	int widthStep = TextureCache::getTexture(filePath).getSize().x / frameWidth;
	int heightStep = TextureCache::getTexture(filePath).getSize().y / frameHeight;
	
	m_frames.reserve(static_cast<size_t>(frameWidth) * frameHeight);

	for (int y = 0; y < frameHeight; y++) {
		for (int x = 0; x < frameWidth; x++) {
			sf::IntRect rect(x * widthStep, y * heightStep, x + widthStep, y + heightStep);
			sf::Sprite sprite(TextureCache::getTexture(filePath), rect);
			sprite.setOrigin(sf::Vector2f(sprite.getTextureRect().width / 2.0f, sprite.getTextureRect().height / 2.0f));
			sprite.setPosition(pos);
			sprite.setScale(sf::Vector2f(scale, scale));
			m_frames.push_back(sprite);
		}
	}

	m_frameDelay = frameDelay;
}

void Animation::nextFrame() {
	if (m_ticksToNextFrame == m_frameDelay) {
		m_frameIndex++;
		m_done = false;

		if (m_frameIndex == m_frames.size()) {
			m_frameIndex = 0;
			m_done = true;
		}

		m_ticksToNextFrame = 0;
	}
	else if (m_frameDelay > 0) {
		m_ticksToNextFrame++;
	}
}

void Animation::draw(sf::RenderWindow& window) {
	window.draw(m_frames[m_frameIndex]);
}