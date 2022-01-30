#include "gamepch.h"

#include "Sounds.h"
#include "Debug.h"
#include "Star.h"
#include "Camera.h"

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
		
		DEBUG_PRINT("Added sound " << filePath << " (" << newSoundBuffer.getChannelCount() << " channels)");
		
		return m_sounds[filePath];
	}
	else {
		return m_sounds[filePath];
	}
}

void Sounds::playSound(const std::string& filePath, const sf::Vector2f& pos, float volume, float pitch, Star* star, bool noSpatial) {
	if (m_playingSounds.size() < 200) {
		getSound(filePath);
		GameSound gameSound;

		gameSound.sound.setBuffer(m_soundBuffers[filePath]);
		gameSound.sound.setVolume(volume * m_globalVolume);
		gameSound.sound.setPitch(pitch);

		if (noSpatial) {
			gameSound.sound.setRelativeToListener(true);
			gameSound.sound.setPosition(sf::Vector3f(0.0f, 0.0f, 0.0f));
		}
		else {
			gameSound.sound.setPosition(sf::Vector3f(pos.x, 0.0f, pos.y));
		}

		gameSound.sound.setMinDistance(1000.f);
		gameSound.sound.setAttenuation(10.f);
		gameSound.star = star;

		m_playingSounds.push_back(gameSound);
	}
}

void Sounds::updateSounds(const Player& player, const Camera& camera, const AllianceList& alliances) {
	sf::Listener::setPosition(sf::Vector3f(camera.getPos().x, camera.getZoomFactor() * 100.0f, camera.getPos().y));
	for (auto it = m_playingSounds.begin(); it != m_playingSounds.end();) {
		if (!it->played) {
			if (it->star != nullptr) {
				if (player.getFaction() != -1) {
					// Dont play sound if player isnt spectating and no allies in system
					if (it->star->numAllies(player.getFaction(), alliances) > 0) {
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

void Sounds::playSoundLocal(const std::string& filePath, Star* star, const sf::Vector2f& pos, float volume, float pitch, bool noSpatial) {
	if (star->isLocalViewActive()) {
		playSound(filePath, pos, volume * m_globalVolume, pitch, star, noSpatial);
	}
}