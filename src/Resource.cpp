#include "gamepch.h"
#include "Resource.h"
#include "TOMLCache.h"

Resource::Resource(const std::string& type) {
	this->type = type;
}

std::string Resource::getName() {
	const toml::table& table = TOMLCache::getTable("data/objects/resources.toml");

	return table[type]["name"].value_or("Unnamed");
}