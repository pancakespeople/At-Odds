#pragma once
#include <random>
#include <SFML/Graphics.hpp>

class Random {
public:
	static int randInt(int min, int max);

	static float randFloat(float min, float max);

	static sf::Vector2f randVec(float min, float max);

	static bool randBool();

private:
	static std::mt19937_64& getGen();
};

