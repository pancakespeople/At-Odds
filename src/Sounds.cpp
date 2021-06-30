#include "gamepch.h"

#include "Sounds.h"
#include "Debug.h"
#include "Star.h"

std::unordered_map<std::string, sf::SoundBuffer> Sounds::m_soundBuffers;
std::unordered_map<std::string, sf::Sound> Sounds::m_sounds;
std::list<Sounds::GameSound> Sounds::m_playingSounds;
float Sounds::m_globalVolume = 1.0f;

sf::Sound& Sounds::getSound(const std::string& filePath) {
	if (m_sounds.count(filePath) == 0) {
		sf::SoundBuffer newSoundBuffer;
		sf::Sound newSound;
		
		if (!newSoundBuffer.loadFromFile(filePath)) {
			DEBUG_PRINT("Failed to load sound " << filePath);
			m_sounds[filePath] = newSound;
			return m_sounds[filePath];
		}

		m_soundBuffers[filePath] = newSoundBuffer;
		newSound.setBuffer(m_soundBuffers[filePath]);
		m_sounds[filePath] = newSound;
		
		DEBUG_PRINT("Added sound " << filePath);
		
		return m_sounds[filePath];
	}
	else {
		return m_sounds[filePath];
	}
}

void Sounds::playSound(const std::string& filePath, float volume, float pitch, Star* star) {
	getSound(filePath);
	GameSound gameSound;

	gameSound.sound.setBuffer(m_soundBuffers[filePath]);
	gameSound.sound.setVolume(volume * m_globalVolume);
	gameSound.sound.setPitch(pitch);
	gameSound.star = star;

	m_playingSounds.push_back(gameSound);
}

void Sounds::updateSounds(const Player& player) {
	for (auto it = m_playingSounds.begin(); it != m_playingSounds.end();) {
		if (!it->played) {
			if (it->star != nullptr) {
				if (player.getFaction() != -1) {
					// Dont play sound if player isnt spectating and no allies in system
					if (it->star->numAllies(player.getFaction()) > 0) {
						// Play sound if allies in system
						it->sound.play();
						it->played = true;
						it++;
					}
					else {
						// No allies, don't play
						it->played = true;
						it++;
					}
				}
				else {
					// Play if spectating
					it->sound.play();
					it->played = true;
					it++;
				}
			}
			else {
				// Play if no star specified (global sound)
				it->sound.play();
				it->played = true;
				it++;
			}
		}
		else if (it->sound.getStatus() == sf::Sound::Status::Stopped && it->played) {
			it = m_playingSounds.erase(it);
		}
		else {
			it++;
		}
	}
}

void Sounds::playSoundLocal(const std::string& filePath, Star* star, float volume, float pitch) {
	if (star->isLocalViewActive()) {
		playSound(filePath, volume * m_globalVolume, pitch, star);
	}
}