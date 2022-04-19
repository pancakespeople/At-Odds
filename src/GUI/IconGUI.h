#pragma once

class Star;
class Renderer;
class Camera;

class IconGUI {
public:
	IconGUI();

	void draw(sf::RenderWindow& window, const Renderer& renderer, Star* currentStar, const Camera& camera);

private:
	sf::Sprite m_sprite;
	sf::Text m_text;
	int m_tooltipLevel = 0;

	void drawTooltip(sf::RenderWindow& window, const std::string& text);

};