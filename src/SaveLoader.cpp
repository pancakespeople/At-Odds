#include "gamepch.h"
#include "SaveLoader.h"
#include "Constellation.h"
#include "GameState.h"

void SaveLoader::saveGame(std::string filePath, const Constellation& constellation, const GameState& state) {
	std::ofstream file(filePath);
	boost::archive::text_oarchive archive(file);

	archive << constellation;
	archive << state;
}

void SaveLoader::loadGame(std::string filePath, Constellation& constellation, GameState& state) {
	std::ifstream file(filePath);
	boost::archive::text_iarchive archive(file);

	archive >> constellation;
	archive >> state;
}