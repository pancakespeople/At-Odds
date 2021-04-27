#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

class TextureCache {
public:
	TextureCache() = delete;
	
	static sf::Texture& getTexture(const std::string& filePath);

private:
	static std::unordered_map<std::string, sf::Texture> m_textures;
};

