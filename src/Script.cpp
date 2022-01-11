#include "gamepch.h"
#include "Script.h"
#include "Random.h"
#include "Projectile.h"
#include "Star.h"
#include "Spaceship.h"

void addUserTypes(sol::state& lua) {
	// Random number generator stuff
	lua.create_table("Random");
	lua["Random"]["randInt"] = &Random::randInt;
	lua["Random"]["randFloat"] = &Random::randFloat;
	lua["Random"]["randBool"] = &Random::randBool;
	lua["Random"]["randVec"] = &Random::randVec;

	// Projectile
	lua.new_usertype<Projectile>("Projectile",
		sol::constructors<Projectile(const std::string&)>(),
		"setPos", &Projectile::setPos,
		"setRotation", &Projectile::setRotation,
		"setAllegiance", &Projectile::setAllegiance
	);

	// Star
	lua.new_usertype<Star>("Star",
		"addProjectile", &Star::addProjectile
	);

	// Spaceship
	lua.new_usertype<Spaceship>("Spaceship",
		"getPos", &Spaceship::getPos,
		"getAllegiance", &Spaceship::getAllegiance
	);
}

bool Script::RunScript(const std::string& filePath) {
	if (!initialized) {
		// Initialize script engine if not initialized
		lua.open_libraries(sol::lib::base);
		addUserTypes(Script::lua);
		initialized = true;
		DEBUG_PRINT("Initialized lua engine");
	}

	// Run script
	auto result = lua.script_file("data/scripts/" + filePath);
	
	if (!result.valid()) {
		DEBUG_PRINT("Script error in " << filePath);
		for (auto& error : result) {
			DEBUG_PRINT(error.as<std::string>());
		}
	}
	
	return result.valid();
}
