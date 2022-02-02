#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

class Renderer;

class Animation {
public:
	Animation(const std::string& type, sf::Vector2f pos);
	Animation(const std::string& filePath, int frameWidth, int frameHeight, sf::Vector2f pos, int frameDelay = 0, float scale = 1.0f);

	void draw(Renderer& renderer);
	void nextFrame();

	bool isDone() { return m_done; }

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

class EffectAnimation {
public:
	enum class Effect {
		NONE,
		LIGHTNING,
		JUMP
	};

	EffectAnimation(Effect effect, sf::Vector2f pos, int ticksAlive);
	void draw(Renderer& renderer);
	void update();

	bool isDone() { return m_done; }

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_type;
		archive & m_pos;
		archive & m_endTimer;
		archive & m_updatesAlive;
		archive & m_done;
		archive & m_seed;
	}

	EffectAnimation() {}

	Effect m_type = Effect::NONE;
	sf::Vector2f m_pos;
	int m_endTimer = 0;
	int m_updatesAlive = 0;
	bool m_done = false;
	float m_seed = 0.0f;
};