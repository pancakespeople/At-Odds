#include "gamepch.h"

#include "Random.h"

std::mt19937_64& Random::getGen() {
	static bool isInit = false;
	static std::mt19937_64 generator;
	if (!isInit) {
		std::random_device rd;
		generator = std::mt19937_64(rd());
		isInit = true;
		DEBUG_PRINT("Initialized random number generator");
	}
	return generator;
}
\
int Random::randInt(int min, int max) {
	std::uniform_int_distribution<int> distr(min, max);
	return distr(getGen());
}

float Random::randFloat(float min, float max) {
	std::uniform_real_distribution<float> distr(min, max);
	return distr(getGen());
}

sf::Vector2f Random::randVec(float min, float max) {
	std::uniform_real_distribution<float> distr(min, max);
	return sf::Vector2f(distr(getGen()), distr(getGen()));
}

bool Random::randBool() {
	std::uniform_int_distribution<int> distr(0, 1);
	return distr(getGen());
}