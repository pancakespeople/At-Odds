#pragma once
#include <SFML/Window.hpp>
#include <unordered_map>

class Keybindings {
public:
	struct Key {
		sf::Keyboard::Key key;
		bool control = false;
		bool shift = false;
		bool alt = false;
		bool system = false;

		Key(sf::Keyboard::Key key, bool control = false, bool shift = false, bool alt = false, bool system = false) {
			this->key = key;
			this->control = control;
			this->alt = alt;
			this->system = system;
		}

		Key() = default;
	};

	static bool isKeyPress(const std::string& keybind, const sf::Event& ev);
	static bool isKeyRelease(const std::string& keybind, const sf::Event& ev);
	static void addDefaultKeybindings();
	static sf::Keyboard::Key getKey(const std::string& keybind);

private:
	inline static std::unordered_map<std::string, Key> m_keybinds;
};