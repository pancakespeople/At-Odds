#include "gamepch.h"
#include "MusicPlayer.h"
#include "Random.h"

#include <filesystem>

MusicPlayer::MusicPlayer() {
	for (auto& file : std::filesystem::directory_iterator("data\\music")) {
		if (file.path().extension() == ".ogg") {
			m_musicFilePathsUnplayed.push_back(file.path().string());
		}
	}
}

void MusicPlayer::playMusic() {
	if (m_music.getStatus() == sf::SoundSource::Stopped) {
		if (m_musicFilePathsUnplayed.size() == 0) {
			if (m_musicFilePathsPlayed.size() > 0) {
				// Reset played tracks
				m_musicFilePathsUnplayed = m_musicFilePathsPlayed;
				m_musicFilePathsPlayed.clear();
			}
		}
		else {
			int randIndex = Random::randInt(0, Random::randInt(0, m_musicFilePathsUnplayed.size() - 1));

			if (!m_music.openFromFile(m_musicFilePathsUnplayed[randIndex])) {
				DEBUG_PRINT("Failed to open music track: " << m_musicFilePathsUnplayed[randIndex]);
				m_musicFilePathsUnplayed.erase(m_musicFilePathsUnplayed.begin() + randIndex);
			}
			else {
				m_music.play();

				m_musicFilePathsPlayed.push_back(m_musicFilePathsUnplayed[randIndex]);
				m_musicFilePathsUnplayed.erase(m_musicFilePathsUnplayed.begin() + randIndex);
			}
		}

	}
}