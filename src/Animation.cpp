#include "gamepch.h"

#include "Animation.h"
#include "TextureCache.h"
#include "TOMLCache.h"
#include "Renderer.h"
#include "Random.h"

Animation::Animation(const std::string& type, sf::Vector2f pos) {
	const toml::table& table = TOMLCache::getTable("data/objects/animations.toml");
	
	assert(table.contains(type));

	std::string filePath = table[type]["filePath"].value_or("");
	int frameWidth = table[type]["frameWidth"].value_or(0);
	int frameHeight = table[type]["frameHeight"].value_or(0);
	int frameDelay = table[type]["frameDelay"].value_or(0);
	float scale = table[type]["scale"].value_or(1.0f);
	
	init(filePath, frameWidth, frameHeight, pos, frameDelay, scale);
}

Animation::Animation(const std::string& filePath, int frameWidth, int frameHeight, sf::Vector2f pos, int frameDelay, float scale) {
	init(filePath, frameWidth, frameHeight, pos, frameDelay, scale);
}

void Animation::init(const std::string& filePath, int frameWidth, int frameHeight, sf::Vector2f pos, int frameDelay, float scale) {
	int widthStep = TextureCache::getTexture(filePath).getSize().x / frameWidth;
	int heightStep = TextureCache::getTexture(filePath).getSize().y / frameHeight;

	m_frames.reserve(static_cast<size_t>(frameWidth) * frameHeight);

	for (int y = 0; y < frameHeight; y++) {
		for (int x = 0; x < frameWidth; x++) {
			sf::IntRect rect(x * widthStep, y * heightStep, widthStep, heightStep);
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

void Animation::draw(Renderer& renderer) {
	renderer.draw(m_frames[m_frameIndex]);
}

EffectAnimation::EffectAnimation(Effect effect, sf::Vector2f pos, int updatesAlive) {
	m_type = effect;
	m_pos = pos;
	m_updatesAlive = updatesAlive;
	m_endTimer = updatesAlive;
	m_seed = Random::randFloat(0.0f, 1.0f);
}

void EffectAnimation::draw(Renderer& renderer) {
	if (!m_done) {
		float time = (m_updatesAlive - m_endTimer) / 60.0f;
		//DEBUG_PRINT(time);

		switch (m_type) {
		case Effect::LIGHTNING:
			renderer.effects.drawLightningEffect(m_pos, time, m_seed);
			break;
		}
	}
}

void EffectAnimation::update() {
	if (m_endTimer == 0) {
		m_done = true;
	}
	else {
		m_endTimer--;
	}
}