#include "gamepch.h"
#include "Fonts.h"

std::unordered_map<std::string, sf::Font> Fonts::m_fonts;

sf::Font& Fonts::getFont(const std::string& filePath) {
	if (m_fonts.count(filePath) == 0) {
		sf::Font newFont;

		if (!newFont.loadFromFile(filePath)) {
			DEBUG_PRINT("Failed to load font " << filePath);
			m_fonts[filePath] = newFont;
			return m_fonts[filePath];
		}

		m_fonts[filePath] = newFont;
		DEBUG_PRINT("Added font " << filePath);

		return m_fonts[filePath];
	}
	else {
		return m_fonts[filePath];
	}
}