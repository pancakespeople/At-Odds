#pragma once

class Fonts {
public:
	Fonts() = delete;

	static sf::Font& getFont(const std::string& filePath);
	static sf::Font& getMainFont() { return getFont(MAIN_FONT_PATH); }

	inline static const auto MAIN_FONT_PATH = "data/fonts/NotoSans-Regular.ttf";

private:
	static std::unordered_map<std::string, sf::Font> m_fonts;
};