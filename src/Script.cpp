#include "gamepch.h"
#include "Script.h"
#include "Random.h"
#include "Projectile.h"
#include "Star.h"
#include "Spaceship.h"
#include "Math.h"
#include "Renderer.h"

void addUserTypes(sol::state& lua) {
	// SFML stuff
	auto sfTable = lua.create_table("sf");
	sfTable.new_usertype<sf::Vector2f>("Vector2f",
		sol::constructors<sf::Vector2<float>(), sf::Vector2<float>(float, float)>(),
		"x", &sf::Vector2f::x,
		"y", &sf::Vector2f::y
	);
	sfTable.new_usertype<sf::RectangleShape>("RectangleShape",
		sol::constructors<sf::RectangleShape()>(),
		sol::base_classes, sol::bases<sf::Shape, sf::Drawable, sf::Transformable>(),
		"setRotation", &sf::RectangleShape::setRotation,
		"setPosition", static_cast<void (sf::RectangleShape::*)(const sf::Vector2f&)>(&sf::RectangleShape::setPosition),
		"setSize", &sf::RectangleShape::setSize,
		"setFillColor", &sf::RectangleShape::setFillColor
	);
	sfTable.new_usertype<sf::Color>("Color",
		sol::constructors<sf::Color(), sf::Color(float, float, float, float), sf::Color(sf::Uint8, sf::Uint8, sf::Uint8, sf::Uint8)>(),
		"r", &sf::Color::r,
		"g", &sf::Color::g,
		"b", &sf::Color::b,
		"a", &sf::Color::a
	);
	
	// Random number generator stuff
	lua.create_table("Random");
	lua["Random"]["randInt"] = &Random::randInt;
	lua["Random"]["randFloat"] = &Random::randFloat;
	lua["Random"]["randBool"] = &Random::randBool;
	lua["Random"]["randVec"] = &Random::randVec;

	// Math
	lua.create_table("Math");
	lua["Math"]["pi"] = Math::pi;
	lua["Math"]["toRadians"] = Math::toRadians;
	lua["Math"]["toDegrees"] = Math::toDegrees;
	lua["Math"]["distance"] = &Math::distance;
	lua["Math"]["angleBetween"] = &Math::angleBetween;
	lua["Math"]["magnitude"] = &Math::magnitude;
	lua["Math"]["lerp"] = &Math::lerp;

	// Renderer
	lua.new_usertype<Renderer>("Renderer",
		"draw", static_cast<void (Renderer::*)(const sf::Drawable&)>(&Renderer::draw)
	);

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
