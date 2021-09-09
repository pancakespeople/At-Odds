#include "gamepch.h"
#include "TOMLCache.h"

std::unordered_map<std::string, toml::table> TOMLCache::m_tables;

const toml::table& TOMLCache::getTable(const std::string& filePath) {
	if (m_tables.count(filePath) == 0) {
		toml::table table;
		try {
			table = toml::parse_file(filePath);
		}
		catch (const toml::parse_error& err) {
			DEBUG_PRINT("Failed to parse TOML file " << filePath << ":\n" << err);
			assert(false);
			m_tables[filePath] = table;
			return m_tables[filePath];
		}
		
		DEBUG_PRINT("Loaded TOML file " << filePath);

		m_tables[filePath] = table;
		return m_tables[filePath];
	}
	else {
		return m_tables[filePath];
	}
}