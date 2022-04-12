#pragma once
#include <vector>
#include <string>
#include <SFML/Audio.hpp>

class MusicPlayer {
public:
	MusicPlayer();

	void update(float currentTimeSeconds);
	void setVolume(float volume) { m_volume = volume; }
	void startMusic(float currentTimeSeconds);

private:
	std::vector<std::string> m_musicFilePathsUnplayed;
	std::vector<std::string> m_musicFilePathsPlayed;
	sf::Music m_music;
	
	bool m_paused = true;
	float m_volume = 0.0f;
	float m_fadeInTimeSeconds;
};