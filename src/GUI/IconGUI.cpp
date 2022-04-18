#include "gamepch.h"
#include "IconGUI.h"
#include "gamepch.h"
#include "IconGUI.h"
#include "../Star.h"
#include "../Renderer.h"

void IconGUI::draw(sf::RenderWindow& window, const Renderer& renderer, Star* currentStar, const Camera& camera) {
	if (currentStar != nullptr && camera.getZoomFactor() > 50.0f) {
		if (currentStar->isDrawingHidden()) {

			sf::Sprite sprite;
			sprite.setOrigin({ 8, 8 });
			sprite.setScale({ 2.0f, 2.0f });

			for (Planet& planet : currentStar->getPlanets()) {
				sf::Vector2i screenPos = renderer.mapCoordsToPixel(planet.getPos());
				std::string texturePath = "data/art/";

				switch (planet.getType()) {
				case Planet::PLANET_TYPE::BARREN:
					texturePath += "barrenicon.png";
					break;
				case Planet::PLANET_TYPE::DESERT:
					texturePath += "deserticon.png";
					break;
				case Planet::PLANET_TYPE::GAS_GIANT:
					texturePath += "gasgianticon.png";
					break;
				case Planet::PLANET_TYPE::ICE_GIANT:
					texturePath += "icegianticon.png";
					break;
				case Planet::PLANET_TYPE::LAVA:
				case Planet::PLANET_TYPE::VOLCANIC:
					texturePath += "lavaicon.png";
					break;
				case Planet::PLANET_TYPE::OCEAN:
					texturePath += "oceanicon.png";
					break;
				case Planet::PLANET_TYPE::TERRA:
					texturePath += "terraicon.png";
					break;
				case Planet::PLANET_TYPE::TOXIC:
					texturePath += "toxicicon.png";
					break;
				case Planet::PLANET_TYPE::TUNDRA:
					texturePath += "tundraicon.png";
					break;
				default:
					texturePath += "barrenicon.png";
				}

				sprite.setTexture(TextureCache::getTexture(texturePath));
				sprite.setPosition(sf::Vector2f(screenPos));
				window.draw(sprite);
			}

			for (JumpPoint& jp : currentStar->getJumpPoints()) {
				sf::Vector2i screenPos = renderer.mapCoordsToPixel(jp.getPos());
				sprite.setTexture(TextureCache::getTexture("data/art/jumppointicon.png"));
				sprite.setPosition(sf::Vector2f(screenPos));
				window.draw(sprite);
			}

		}
	}
}
