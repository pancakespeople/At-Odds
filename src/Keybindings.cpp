#include "gamepch.h"
#include "Keybindings.h"

bool Keybindings::isKeyPress(const std::string& keybind, const sf::Event& ev) {
	if (m_keybinds.find(keybind) != m_keybinds.end() && ev.type == sf::Event::KeyPressed) {
		Key& key = m_keybinds.at(keybind);
		if (key.key == ev.key.code &&
			key.control == ev.key.control &&
			key.shift == ev.key.shift &&
			key.alt == ev.key.alt &&
			key.system == ev.key.system) {
			return true;
		}
	}
	return false;
}

void Keybindings::addDefaultKeybindings() {
	m_keybinds["CameraLeft"] = Key(sf::Keyboard::A);
	m_keybinds["CameraUp"] = Key(sf::Keyboard::W);
	m_keybinds["CameraDown"] = Key(sf::Keyboard::S);
	m_keybinds["CameraRight"] = Key(sf::Keyboard::D);

	m_keybinds["AltCameraLeft"] = Key(sf::Keyboard::Left);
	m_keybinds["AltCameraUp"] = Key(sf::Keyboard::Up);
	m_keybinds["AltCameraDown"] = Key(sf::Keyboard::Down);
	m_keybinds["AltCameraRight"] = Key(sf::Keyboard::Right);

	m_keybinds["SwitchView"] = Key(sf::Keyboard::Tab);
	m_keybinds["MainMenu"] = Key(sf::Keyboard::Escape);
	
	m_keybinds["SelectAllCombatUnitsInSystem"] = Key(sf::Keyboard::A, true);
}

sf::Keyboard::Key Keybindings::getKey(const std::string& keybind) {
	return m_keybinds.at(keybind).key;
}