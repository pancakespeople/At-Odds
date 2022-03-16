#pragma once
#include <string>
#include <unordered_map>
#include <boost/serialization/access.hpp>

struct DesignerChassis {
	DesignerChassis() {}
	DesignerChassis(const std::string& typeStr);

	std::string type;
	std::string name;
	std::unordered_map<std::string, float> resourceCost;
	float maxWeaponCapacity = 1.0f;
	float buildTimeMultiplier = 1.0f;
	bool miningChassis = false;
	bool constructionChassis = false;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & type;
		archive & name;
		archive & maxWeaponCapacity;
		archive & resourceCost;
		archive & buildTimeMultiplier;
	}
};

struct DesignerWeapon {
	DesignerWeapon() {}
	DesignerWeapon(const std::string& typeStr);

	std::string getFullName();

	std::string name;
	std::string type;
	std::unordered_map<std::string, float> resourceCost;
	float weaponPoints = 1.0f;
	int upgradeLevel = 1;
	bool miningWeapon = false;
	bool constructionWeapon = false;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & type;
		archive & name;
		archive & weaponPoints;
		archive & resourceCost;
		archive & upgradeLevel;
	}

};

struct DesignerShip {
	DesignerChassis chassis;
	std::vector<DesignerWeapon> weapons;
	std::string name;

	std::unordered_map<std::string, float> getTotalResourceCost();
	std::string generateName();
	float getTotalWeaponPoints();

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & chassis;
		archive & weapons;
		archive & name;
	}
};