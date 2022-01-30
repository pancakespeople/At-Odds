#pragma once
#include <unordered_map>
#include <SFML/Audio.hpp>
#include <string>
#include <list>

#include "GUI.h"

class Star;
class Player;
class Camera;
class AllianceList;

class Sounds {
public:
	struct GameSound {
		sf::Sound sound;
		bool played = false;
		Star* star = nullptr;
	};
	
	static void playSound(const std::string& filePath, const sf::Vector2f& pos, float volume = 100.0f, float pitch = 1.0f, Star* star = nullptr, bool noSpatial = false);
	static void playSoundLocal(const std::string& filePath, Star* star, const sf::Vector2f& pos, float volume = 100.0f, float pitch = 1.0f, bool noSpatial = false);
	static void loadSound(const std::string& filepath) { getSound(filepath); }
	static void updateSounds(const Player& player, const Camera& camera, const AllianceList& alliances);
	static void clearSounds() { m_playingSounds.clear(); }

private:
	friend class OptionsMenu;
	
	static sf::Sound& getSound(const std::string& filePath);

	static void setGlobalVolume(float volume) { m_globalVolume = volume; }

	static std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers;
	static std::unordered_map<std::string, sf::Sound> m_sounds;
	static std::list<GameSound> m_playingSounds;
	static float m_globalVolume;
};

