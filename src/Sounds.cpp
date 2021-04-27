#include "gamepch.h"

#include "Sounds.h"
#include "Debug.h"
#include "Star.h"

std::unordered_map<std::string, sf::SoundBuffer> Sounds::m_soundBuffers;
std::unordered_map<std::string, sf::Sound> Sounds::m_sounds;
std::list<sf::Sound> Sounds::m_overflowBuffer;
float Sounds::m_globalVolume = 1.0f;

#ifdef NDEBUG
#define MAX_OVERFLOW_SOUNDS 32
#else
#define MAX_OVERFLOW_SOUNDS 0
#endif

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

void Sounds::playSound(const std::string& filePath, float volume, float pitch) {
	if (getSound(filePath).getStatus() == sf::SoundSource::Playing) {
		if (m_overflowBuffer.size() < MAX_OVERFLOW_SOUNDS) {
			// Clean up overflow buffer
			std::list<sf::Sound>::iterator i = m_overflowBuffer.begin();
			while (i != m_overflowBuffer.end()) {
				if (i->getStatus() == sf::Sound::Stopped) {
					m_overflowBuffer.erase(i++);
				}
				else {
					++i;
				}
			}
			
			sf::Sound overflow;
			m_overflowBuffer.push_back(overflow);
			m_overflowBuffer.back().setBuffer(m_soundBuffers[filePath]);
			m_overflowBuffer.back().setVolume(volume * m_globalVolume);
			m_overflowBuffer.back().setPitch(pitch);
			m_overflowBuffer.back().play();
		}
		else {
			// Clean up overflow buffer
			std::list<sf::Sound>::iterator i = m_overflowBuffer.begin();
			while (i != m_overflowBuffer.end()) {
				if (i->getStatus() == sf::Sound::Stopped) {
					m_overflowBuffer.erase(i++);
				}
				else {
					++i;
				}
			}
		}
	}
	else {
		sf::Sound& sound = getSound(filePath);
		sound.setVolume(volume * m_globalVolume);
		sound.setPitch(pitch);
		sound.play();
	}
}

void Sounds::playSoundLocal(const std::string& filePath, Star* star, float volume, float pitch) {
	if (star->isLocalViewActive()) {
		playSound(filePath, volume * m_globalVolume, pitch);
	}
}