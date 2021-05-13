#include "gamepch.h"
#include "SaveLoader.h"
#include "Constellation.h"
#include "GameState.h"
#include "Identifiable.h"

void SaveLoader::saveGame(std::string filePath, const Constellation& constellation, const GameState& state) {
	std::ofstream file(filePath, std::ios::binary);
	boost::archive::binary_oarchive archive(file);

	archive << constellation;
	archive << Identifiable::numObjects;
	archive << state;
}

void SaveLoader::loadGame(std::string filePath, Constellation& constellation, GameState& state) {
	std::ifstream file(filePath, std::ios::binary);
	boost::archive::binary_iarchive archive(file);

	archive >> constellation;
	archive >> Identifiable::numObjects;
	archive >> state;
}