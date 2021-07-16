#pragma once
#include "Spaceship.h"

// Ready made ship parts
namespace ShipParts {
	const Spaceship::DesignerChassis frigateChassis = {
		"FRIGATE_1", // Type
		"Frigate", // Name
		1.0f, // Max weapon capacity
		{{PlanetResource::RESOURCE_TYPE::COMMON_ORE, 7.5f} } // Resource costs
	};

	const Spaceship::DesignerChassis destroyerChassis = {
		"DESTROYER_1",
		"Destroyer",
		2.0f,
		{{PlanetResource::RESOURCE_TYPE::COMMON_ORE, 15.0f}}
	};

	const Spaceship::DesignerChassis constructorChassis = {
		"CONSTRUCTION_SHIP",
		"Constructor",
		0.0f,
		{{PlanetResource::RESOURCE_TYPE::COMMON_ORE, 33.0f}}
	};

	const Spaceship::DesignerWeapon laserGunWeapon = {
		Weapon::WEAPON_TYPE::LASER_GUN,
		"Laser Gun",
		1.0f,
		{{PlanetResource::RESOURCE_TYPE::COMMON_ORE, 2.5f}}
	};

	const Spaceship::DesignerWeapon machineGunWeapon = {
		Weapon::WEAPON_TYPE::MACHINE_GUN,
		"Machine Gun",
		1.0f,
		{{PlanetResource::RESOURCE_TYPE::COMMON_ORE, 2.5f}}
	};

	const Spaceship::DesignerWeapon gaussCannonWeapon = {
		Weapon::WEAPON_TYPE::GAUSS_CANNON,
		"Gauss Cannon",
		2.0f,
		{{PlanetResource::RESOURCE_TYPE::COMMON_ORE, 10.0f}, {PlanetResource::RESOURCE_TYPE::UNCOMMON_ORE, 5.0f}}
	};
}