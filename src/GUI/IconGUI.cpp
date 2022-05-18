#include "gamepch.h"
#include "IconGUI.h"
#include "gamepch.h"
#include "IconGUI.h"
#include "../Star.h"
#include "../Renderer.h"
#include "../Fonts.h"

IconGUI::IconGUI() {
	m_sprite.setOrigin({ 8, 8 });
	m_sprite.setScale({ 2.0f, 2.0f });

	m_text.setFont(Fonts::getMainFont());
	m_text.setString("Test Text");
	m_text.setOrigin({ 0.0f, 50.0f });
}

void IconGUI::draw(sf::RenderWindow& window, const Renderer& renderer, Star* currentStar, const Camera& camera) {
	if (currentStar != nullptr && camera.getZoomFactor() > 50.0f) {
		if (currentStar->isDrawingHidden()) {
			m_tooltipLevel = 0;

			for (Planet& planet : currentStar->getPlanets()) {
				sf::Vector2i screenPos = renderer.mapCoordsToPixel(planet.getPos());
				std::string texturePath = "data/art/";
				std::string tooltipText;

				switch (planet.getType()) {
				case Planet::PLANET_TYPE::BARREN:
					texturePath += "barrenicon.png";
					tooltipText = "Barren Planet";
					break;
				case Planet::PLANET_TYPE::DESERT:
					texturePath += "deserticon.png";
					tooltipText = "Desert Planet";
					break;
				case Planet::PLANET_TYPE::GAS_GIANT:
					texturePath += "gasgianticon.png";
					tooltipText = "Gas Giant Planet";
					break;
				case Planet::PLANET_TYPE::ICE_GIANT:
					texturePath += "icegianticon.png";
					tooltipText = "Ice Giant Planet";
					break;
				case Planet::PLANET_TYPE::LAVA:
					texturePath += "lavaicon.png";
					tooltipText = "Lava Planet";
					break;
				case Planet::PLANET_TYPE::VOLCANIC:
					texturePath += "lavaicon.png";
					tooltipText = "Volcanic Planet";
					break;
				case Planet::PLANET_TYPE::OCEAN:
					texturePath += "oceanicon.png";
					tooltipText = "Ocean Planet";
					break;
				case Planet::PLANET_TYPE::TERRA:
					texturePath += "terraicon.png";
					tooltipText = "Terra Planet";
					break;
				case Planet::PLANET_TYPE::TOXIC:
					texturePath += "toxicicon.png";
					tooltipText = "Toxic Planet";
					break;
				case Planet::PLANET_TYPE::TUNDRA:
					texturePath += "tundraicon.png";
					tooltipText = "Tundra Planet";
					break;
				default:
					texturePath += "barrenicon.png";
					tooltipText = "Unknown Planet";
				}

				m_sprite.setTexture(TextureCache::getTexture(texturePath));
				m_sprite.setPosition(sf::Vector2f(screenPos));
				window.draw(m_sprite);
				drawTooltip(window, tooltipText);
			}

			for (JumpPoint& jp : currentStar->getJumpPoints()) {
				sf::Vector2i screenPos = renderer.mapCoordsToPixel(jp.getPos());
				m_sprite.setTexture(TextureCache::getTexture("data/art/jumppointicon.png"));
				m_sprite.setPosition(sf::Vector2f(screenPos));
				window.draw(m_sprite);
				drawTooltip(window, "Jump Point");
			}

			for (auto& asteroid : currentStar->getAsteroids()) {
				sf::Vector2i screenPos = renderer.mapCoordsToPixel(asteroid->getPos());
				std::string texturePath = "data/art/";
				std::string tooltipText;

				if (asteroid->getResource() == "COMMON_ORE") {
					texturePath += "kathiumicon.png";
					tooltipText = "Kathium Asteroid";
				}
				else if (asteroid->getResource() == "UNCOMMON_ORE") {
					texturePath += "oscilliteicon.png";
					tooltipText = "Oscillite Asteroid";
				}
				else if (asteroid->getResource() == "RARE_ORE") {
					texturePath += "valkicon2.png";
					tooltipText = "Valkrosium Asteroid";
				}

				m_sprite.setTexture(TextureCache::getTexture(texturePath));
				m_sprite.setPosition(sf::Vector2f(screenPos));
				window.draw(m_sprite);
				drawTooltip(window, tooltipText);
			}
		}
	}
}

void IconGUI::drawTooltip(sf::RenderWindow& window, const std::string& text) {
	if (m_sprite.getGlobalBounds().contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
		m_text.setPosition(sf::Vector2f(sf::Mouse::getPosition(window)) - sf::Vector2f(0.0f, 50.0f * m_tooltipLevel));
		m_text.setString(text);
		window.draw(m_text);

		m_tooltipLevel++;
	}
}