#include "gamepch.h"
#include "SaveLoader.h"
#include "Constellation.h"
#include "GameState.h"
#include "Identifiable.h"
#include "Background.h"

void SaveLoader::saveGame(std::string filePath, const Constellation& constellation, const GameState& state, const Background& background) {
	std::ofstream file(filePath, std::ios::binary);
	boost::archive::binary_oarchive archive(file);

	archive << constellation;
	archive << Identifiable::numObjects;
	archive << state;
	archive << background.getNebulaSeed();
}

bool SaveLoader::loadGame(std::string filePath, Constellation& constellation, GameState& state, Background& background) {
	std::ifstream file(filePath, std::ios::binary);
	
	if (file.is_open()) {
		boost::archive::binary_iarchive archive(file);

		archive >> constellation;
		archive >> Identifiable::numObjects;
		archive >> state;

		float nebulaSeed;
		archive >> nebulaSeed;

		background.setNebulaSeed(nebulaSeed);
		constellation.reinitAfterLoad();
		state.reinitAfterLoad(constellation);

		return true;
	}
	return false;
}