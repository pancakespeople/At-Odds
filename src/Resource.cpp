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

std::string Resource::getResourceString(const std::unordered_map<std::string, float>& resources) {
	const toml::table& table = TOMLCache::getTable("data/objects/resources.toml");
	std::stringstream ss;

	int num = 0;
	for (auto& keyVal : resources) {
		num++;
		if (num == resources.size()) {
			ss << table[keyVal.first]["name"].value_or("Unnamed") << ": " << keyVal.second;
		}
		else {
			ss << table[keyVal.first]["name"].value_or("Unnamed") << ": " << keyVal.second << ", ";
		}
	}

	return ss.str();
}

std::unordered_map<std::string, float> Resource::nodeToMap(const toml::node_view<const toml::node>& node) {
	std::unordered_map<std::string, float> map;

	if (node.as_array() != nullptr) {
		for (int i = 0; i < node.as_array()->size(); i++) {
			std::string resourceType = node[i][0].value_or("");
			map[resourceType] = node[i][1].value_or(0.0f);
		}
	}

	return map;
}