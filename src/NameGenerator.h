#pragma once

class NameGenerator {
public:
	NameGenerator();

	std::string generateName();

private:
	std::vector<std::string> m_prefixes;
	std::vector<std::string> m_suffixes;
};