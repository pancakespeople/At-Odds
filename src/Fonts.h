#pragma once

class Fonts {
public:
	Fonts() = delete;

	static sf::Font& getFont(const std::string& filePath);

private:
	static std::unordered_map<std::string, sf::Font> m_fonts;
};