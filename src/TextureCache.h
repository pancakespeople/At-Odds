#pragma once
#include <SFML/Graphics.hpp>
#include <map>

class TextureCache {
public:
	TextureCache() = delete;
	
	static sf::Texture& getTexture(const std::string& filePath);

	static std::string getTexturePath(const sf::Texture* texture);

private:
	static std::map<std::string, sf::Texture> m_textures;
};

