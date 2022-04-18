#pragma once

class Star;
class Renderer;
class Camera;

class IconGUI {
public:
	void draw(sf::RenderWindow& window, const Renderer& renderer, Star* currentStar, const Camera& camera);
};