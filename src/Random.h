#pragma once
#include <random>
#include <SFML/Graphics.hpp>

class Random {
public:
	static int randInt(int min, int max);
	static float randFloat(float min, float max);
	static sf::Vector2f randVec(float min, float max);
	static sf::Vector2f randPointInCircle(float radius);
	static bool randBool();
	static std::string randString(int numChars);
	static std::string getGeneratorState();
	static void setGeneratorState(const std::string& str);
	static void setGeneratorSeed(uint64_t seed);

private:
	static std::mt19937_64& getGen();
};

