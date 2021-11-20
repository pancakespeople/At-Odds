#pragma once
#include "ext/toml.hpp"
#include <unordered_map>

class TOMLCache {
public:
	static const toml::table& getTable(const std::string& filePath);

private:
	static std::unordered_map<std::string, toml::table> m_tables;
};