#include "gamepch.h"
#include "Designs.h"
#include "TOMLCache.h"
#include "Util.h"

std::unordered_map<std::string, float> DesignerShip::getTotalResourceCost() {
	std::unordered_map<std::string, float> totalResourceCost;

	for (auto& resource : chassis.resourceCost) {
		totalResourceCost[resource.first] += resource.second;
	}

	for (DesignerWeapon& weapon : weapons) {
		for (auto& resource : weapon.resourceCost) {
			totalResourceCost[resource.first] += resource.second;
		}
	}

	return totalResourceCost;
}

DesignerChassis::DesignerChassis(const std::string& typeStr) {
	const toml::table& table = TOMLCache::getTable("data/objects/spaceships.toml");

	assert(table.contains(typeStr));

	std::string typeName = table[typeStr]["type"].value_or("");

	type = typeStr;

	if (typeName == "") {
		name = table[typeStr]["name"].value_or("");
	}
	else {
		name = table[typeStr]["name"].value_or(std::string()) + " " + typeName;
	}

	maxWeaponCapacity = table[typeStr]["maxWeaponCapacity"].value_or(1.0f);
	buildTimeMultiplier = table[typeStr]["buildTimeMultiplier"].value_or(1.0f);

	for (int i = 0; i < table[typeStr]["cost"].as_array()->size(); i++) {
		std::string resourceType = table[typeStr]["cost"][i][0].value_or("");
		resourceCost[resourceType] = table[typeStr]["cost"][i][1].value_or(0.0f);
	}
}

DesignerWeapon::DesignerWeapon(const std::string& typeStr) {
	const toml::table& table = TOMLCache::getTable("data/objects/weapons.toml");

	assert(table.contains(typeStr));

	type = typeStr;
	name = table[typeStr]["name"].value_or("");
	weaponPoints = table[typeStr]["weaponPoints"].value_or(1.0f);

	for (int i = 0; i < table[typeStr]["cost"].as_array()->size(); i++) {
		std::string resourceType = table[typeStr]["cost"][i][0].value_or("");
		resourceCost[resourceType] = table[typeStr]["cost"][i][1].value_or(0.0f);
	}
}

std::string DesignerWeapon::getFullName() {
	if (upgradeLevel == 1) {
		return name;
	}
	else {
		return name + " " + Util::toRomanNumeral(upgradeLevel);
	}
}

std::string DesignerShip::generateName() {
	std::string weaponName;
	if (weapons.size() > 0) {
		weaponName = weapons[0].name;
		return weaponName + " " + chassis.name;
	}
	return chassis.name;
}

float DesignerShip::getTotalWeaponPoints() {
	float total = 0.0f;
	for (auto& weapon : weapons) {
		total += weapon.weaponPoints;
	}
	return total;
}
