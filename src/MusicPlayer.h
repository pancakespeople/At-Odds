#pragma once
#include <vector>
#include <string>
#include <SFML/Audio.hpp>

class MusicPlayer {
public:
	MusicPlayer();

	void playMusic();

private:
	std::vector<std::string> m_musicFilePathsUnplayed;
	std::vector<std::string> m_musicFilePathsPlayed;
	sf::Music m_music;
};