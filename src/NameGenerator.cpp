#include "gamepch.h"
#include "NameGenerator.h"
#include "Random.h"

#include <fstream>

NameGenerator::NameGenerator() {
	std::ifstream prefixFile;
	prefixFile.open("data/names/prefixes.txt");

	while (!prefixFile.eof()) {
		std::string prefix;
		prefixFile >> prefix;
		m_prefixes.push_back(prefix);
	}

	std::ifstream suffixFile;
	suffixFile.open("data/names/suffixes.txt");

	while (!suffixFile.eof()) {
		std::string suffix;
		suffixFile >> suffix;
		m_suffixes.push_back(suffix);
	}

	DEBUG_PRINT("Name generator initialized");
}

std::string NameGenerator::generateName() {
	std::string prefix = m_prefixes[Random::randInt(0, m_prefixes.size() - 1)];
	std::string suffix = m_suffixes[Random::randInt(0, m_suffixes.size() - 1)];

	prefix[0] = std::toupper(prefix[0]);

	return prefix + suffix;
}