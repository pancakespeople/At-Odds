#include "gamepch.h"

#include "TextureCache.h"
#include "Debug.h"

std::unordered_map<std::string, sf::Texture> TextureCache::m_textures;

sf::Texture& TextureCache::getTexture(const std::string& filePath) {
	if (m_textures.count(filePath) == 0) {
		sf::Texture newTexture;
		
		if (!newTexture.loadFromFile(filePath)) {
			DEBUG_PRINT("Failed to load texture " << filePath);
			m_textures[filePath] = newTexture;
			return m_textures[filePath];
		}
		
		m_textures[filePath] = newTexture;
		DEBUG_PRINT("Added texture " << filePath);

		return m_textures[filePath];
	}
	else {
		return m_textures[filePath];
	}
}