#include "gamepch.h"
#include "MusicPlayer.h"
#include "Random.h"
#include "Math.h"

#include <filesystem>

MusicPlayer::MusicPlayer() {
	for (auto& file : std::filesystem::directory_iterator("data\\music")) {
		if (file.path().extension() == ".ogg") {
			m_musicFilePathsUnplayed.push_back(file.path().string());
		}
	}

	m_music.setVolume(0.0f);
}

void MusicPlayer::update(float currentTimeSeconds) {
	if (!m_paused) {
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
		else {
			// Fade in music on start
			float fadeInTime = currentTimeSeconds - m_fadeInTimeSeconds;
			m_music.setVolume(Math::clamp(fadeInTime / 10.0f, 0.0f, 1.0f) * m_volume);
		}
	}
}

void MusicPlayer::startMusic(float currentTimeSeconds) {
	m_paused = false;
	m_fadeInTimeSeconds = currentTimeSeconds;
}