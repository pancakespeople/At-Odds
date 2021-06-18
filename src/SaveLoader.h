#pragma once
#include <SFML/Graphics.hpp>
#include <string>

#include "TextureCache.h"

class Constellation;
class GameState;
class Background;

namespace boost {
	namespace serialization {
		
		// sf::View
		template<class Archive>
		void save(Archive& archive, const sf::View& view, const unsigned int version) {
			archive& view.getCenter();
			archive& view.getRotation();
			archive& view.getSize();
		}

		template<class Archive>
		void load(Archive& archive, sf::View& view, const unsigned int version) {
			sf::Vector2f center;
			float rotation;
			sf::Vector2f size;

			archive& center;
			archive& rotation;
			archive& size;

			view.setCenter(center);
			view.setRotation(rotation);
			view.setSize(size);
		}

		// sf::Transformable
		template<class Archive>
		void save(Archive& archive, const sf::Transformable& shape, const unsigned int version) {
			archive& shape.getPosition();
			archive& shape.getOrigin();
			archive& shape.getRotation();
			archive& shape.getScale();
		}

		template<class Archive>
		void load(Archive& archive, sf::Transformable& shape, const unsigned int version) {
			sf::Vector2f pos;
			sf::Vector2f origin;
			float rotation;
			sf::Vector2f scale;

			archive& pos;
			archive& origin;
			archive& rotation;
			archive& scale;

			shape.setPosition(pos);
			shape.setOrigin(origin);
			shape.setRotation(rotation);
			shape.setScale(scale);
		}

		// sf::Shape
		template<class Archive>
		void save(Archive& archive, const sf::Shape& shape, const unsigned int version) {
			archive& boost::serialization::base_object<sf::Transformable>(shape);
			archive& shape.getFillColor();
			archive& shape.getOutlineColor();
			archive& shape.getOutlineThickness();
		}

		template<class Archive>
		void load(Archive& archive, sf::Shape& shape, const unsigned int version) {
			sf::Color fillColor;
			sf::Color outlineColor;
			float outlineThickness;

			archive& boost::serialization::base_object<sf::Transformable>(shape);
			archive& fillColor;
			archive& outlineColor;
			archive& outlineThickness;

			shape.setFillColor(fillColor);
			shape.setOutlineColor(outlineColor);
			shape.setOutlineThickness(outlineThickness);
		}

		// sf::CircleShape
		template<class Archive>
		void save(Archive& archive, const sf::CircleShape& shape, const unsigned int version) {
			archive& boost::serialization::base_object<sf::Shape>(shape);
			archive& shape.getRadius();
			archive& shape.getPointCount();
		}

		template<class Archive>
		void load(Archive& archive, sf::CircleShape& shape, const unsigned int version) {
			float radius;
			size_t pointCount;

			archive& boost::serialization::base_object<sf::Shape>(shape);
			archive& radius;
			archive& pointCount;

			shape.setRadius(radius);
			shape.setPointCount(pointCount);
		}

		// sf::Sprite
		template<class Archive>
		void save(Archive& archive, const sf::Sprite& sprite, const unsigned int version) {
			archive& boost::serialization::base_object<sf::Transformable>(sprite);
			archive& TextureCache::getTexturePath(sprite.getTexture());
			archive& sprite.getColor();
		}

		template<class Archive>
		void load(Archive& archive, sf::Sprite& sprite, const unsigned int version) {
			std::string filePath;
			sf::Color color;
			
			archive& boost::serialization::base_object<sf::Transformable>(sprite);
			archive& filePath;
			archive& color;

			if (filePath != "") {
				sprite.setTexture(TextureCache::getTexture(filePath));
			}

			sprite.setColor(color);
		}

		// sf::RectangleShape
		template<class Archive>
		void save(Archive& archive, const sf::RectangleShape& shape, const unsigned int version) {
			archive& boost::serialization::base_object<sf::Shape>(shape);
			archive& shape.getSize();
		}

		template<class Archive>
		void load(Archive& archive, sf::RectangleShape& shape, const unsigned int version) {
			sf::Vector2f size;
			
			archive& boost::serialization::base_object<sf::Shape>(shape);
			archive& size;

			shape.setSize(size);
		}
		
		// sf::Rect<float>
		template<class Archive>
		void serialize(Archive& archive, sf::Rect<float>& rect, const unsigned int version) {
			archive& rect.top;
			archive& rect.left;
			archive& rect.width;
			archive& rect.height;
		}

		// sf::Color
		template<class Archive>
		void serialize(Archive& archive, sf::Color& rect, const unsigned int version) {
			archive& rect.r;
			archive& rect.g;
			archive& rect.b;
			archive& rect.a;
		}

		// sf::Vector2<float> 
		template<class Archive>
		void serialize(Archive& archive, sf::Vector2f& vec, const unsigned int version) {
			archive& vec.x;
			archive& vec.y;
		}
	}
}

BOOST_SERIALIZATION_SPLIT_FREE(sf::View)
BOOST_SERIALIZATION_SPLIT_FREE(sf::Transformable)
BOOST_SERIALIZATION_SPLIT_FREE(sf::Shape)
BOOST_SERIALIZATION_SPLIT_FREE(sf::CircleShape)
BOOST_SERIALIZATION_SPLIT_FREE(sf::Sprite)
BOOST_SERIALIZATION_SPLIT_FREE(sf::RectangleShape)

class SaveLoader {
public:
	SaveLoader() {}

	void saveGame(std::string filePath, const Constellation& constellation, const GameState& state, const Background& background);
	void loadGame(std::string filePath, Constellation& constellation, GameState& state, Background& background);
};