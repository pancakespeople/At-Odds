#pragma once
#include <unordered_map>
#include <SFML/Audio.hpp>
#include <string>
#include <list>

#include "GUI.h"

class Star;

class Sounds {
public:
	static void playSound(const std::string& filePath, float volume = 100.0f, float pitch = 1.0f);

	static void playSoundLocal(const std::string& filePath, Star* star, float volume = 100.0f, float pitch = 1.0f);

	static void loadSound(const std::string& filepath) { getSound(filepath); }

private:
	friend class OptionsMenu;
	
	static sf::Sound& getSound(const std::string& filePath);

	static void setGlobalVolume(float volume) { m_globalVolume = volume; }

	static std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers;
	static std::unordered_map<std::string, sf::Sound> m_sounds;
	static std::list<sf::Sound> m_overflowBuffer;
	static float m_globalVolume;
};

