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

std::string Random::getGeneratorState() {
	std::stringstream stream;
	stream << getGen();
	return stream.str();
}

void Random::setGeneratorState(const std::string& str) {
	std::stringstream(str) >> getGen();
}

std::string Random::randString(int numChars) {
	const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
	std::string rand;
	for (int i = 0; i < numChars; i++) {
		rand += chars[randInt(0, chars.length() - 1)];
	}
	return rand;
}

void Random::setGeneratorSeed(uint64_t seed) {
	getGen().seed(seed);
}