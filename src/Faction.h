#pragma once
#include "Spaceship.h"
#include "Brain.h"
#include "Planet.h"
#include "Building.h"
#include "Tech.h"
#include "Designs.h"

class Constellation;
class Player;
class Building;

class Faction {
public:
	Faction(int id);

	static const sf::Color neutralColor;

	void spawnAtRandomStar(Constellation* constellation);
	void addOwnedSystem(Star* star);
	void makeCapital(Star* star);
	void update();
	void controlByPlayer(Player& player);
	void orderConstructionShipsBuild(Building* building, bool onlyIdleShips = false, bool onlyOne = false);
	void addSpaceship(Spaceship* ship) { m_ships.push_back(ship); m_shipIDs.push_back(ship->getID()); }
	void addResource(const std::string& type, float num);
	void subtractResource(const std::string& type, float num);
	void addChassis(const DesignerChassis& chassis) { m_chassis.push_back(chassis); }
	void addWeapon(const DesignerWeapon& weapon) { if (!hasWeapon(weapon.type)) m_weapons.push_back(weapon); }
	void addShipDesign(const DesignerShip& ship);
	void subtractResources(const std::unordered_map<std::string, float>& resources);
	void addAnnouncementEvent(const std::string& text);
	void clearAnnouncementEvents() { m_announcementEvents.clear(); }
	void setColor(sf::Color color) { m_color = color; }
	void setName(const std::string& name) { m_name = name; }
	void reinitAfterLoad(Constellation* constellation);
	void addColonyBuilding(const std::string& type);
	void setResearchingTech(const std::string& type, bool research);
	void onResearchFinish(Tech& tech);
	void addResearchPoints(float points) { m_currentResearchPoints += points; }
	void onStart();
	void setAIEnabled(bool enabled) { m_aiEnabled = enabled; }
	void addNewsEvent(const std::string& text);
	void onColonization(Planet* planet, Star* star);

	int getID() { return m_id; }
	int numUnbuiltBuildings(Star* star);
	int numIdleConstructionShips();
	int getScienceLabMax(Star* star) const;
	int getNumTicksAlive() const { return m_numTicksAlive; }

	float getResourceCount(const std::string& type) const;
	float getResourceExploitation(const std::string& type) const;
	float getResearchPointProduction() const { return m_researchPointProduction; }

	bool isDead() { return m_dead; }
	// Returns false if unsuccessful (if subtracting would lead to a negative resource amount)
	bool canSubtractResource(const std::string& type, float num);
	bool canSubtractResources(const std::unordered_map<std::string, float>& resources);
	bool hasWeapon(const std::string& type);
	bool hasResearchedTech(const std::string& type) const;
	bool hasColonyBuilding(const std::string& type) const;
	
	const Tech* getTech(const std::string& type) const;

	sf::Color getColor() { return m_color; }

	std::vector<Spaceship*>& getShips() { return m_ships; }
	std::vector<Spaceship*> getConstructionShips(bool onlyIdleShips = false);
	std::vector<Star*>& getOwnedStars() { return m_ownedSystems; }
	std::unordered_map<std::string, float>& getResources() { return m_resources; }
	std::vector<Building*> getAllOwnedBuildings();
	std::vector<Building*> getAllOwnedBuildingsOfType(const std::string& type);
	std::vector<Spaceship*> getAllCombatShips();
	std::vector<DesignerChassis>& getChassis() { return m_chassis; }
	std::vector<DesignerWeapon>& getWeapons() { return m_weapons; }
	std::vector<DesignerShip>& getShipDesigns() { return m_designerShips; }
	std::deque<std::string>& getAnnouncementEvents() { return m_announcementEvents; }
	std::string getName() { return m_name; }
	std::vector<DesignerWeapon> getWeaponsBelowOrEqualWeaponPoints(float wp);
	std::vector<ColonyBuilding> getColonyBuildings();
	std::vector<Planet*> getOwnedPlanets() const;
	std::vector<Star*> getUnderAttackStars();
	const std::vector<Tech>& getTechs() { return m_techs; }
	std::vector<Tech> getUnresearchedTechs();
	std::vector<Tech> getAllTechsOfCategory(const std::string& category, bool unresearchedOnly = false);
	std::deque<std::string>& getNewsEvents() { return m_newsEvents; }
	
	// Gets stars that are connected to this faction's owned stars
	std::vector<Star*> getBorderStars();
	
	// Gets ships that can attack planets
	std::vector<Spaceship*> getPlanetAttackShips();
	
	// Gets weapons that are allowed to be fitted onto buildings
	std::vector<DesignerWeapon> getBuildingWeapons();
	
	// Gets all enemy planets that are in this faction's territory
	std::vector<Planet*> getEnemyPlanets();

	DesignerChassis getChassisByName(const std::string& name);
	DesignerWeapon getWeaponByName(const std::string& name);
	DesignerShip getShipDesignByName(const std::string& name);
	DesignerWeapon addRandomWeapon();
	DesignerWeapon addRandomUndiscoveredWeapon();

	Star* getCapital() { return m_capital; }
	Star* getRandomOwnedStar();

	Planet* getMostHabitablePlanet();

	template <typename T>
	void giveAllShipsOrder(const T order) {
		for (Spaceship* s : m_ships) {
			s->addOrder(order);
		}
	}

	template<typename T>
	void giveAllCombatShipsOrder(const T order, bool clearOrders = false) {
		for (Spaceship* s : m_ships) {
			if (s->getConstructionSpeed() == 0.0f) {
				if (clearOrders) s->clearOrders();
				s->addOrder(order);
			}
		}
	}

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_capitalID;
		archive & m_color;
		archive & m_ownedSystemIDs;
		archive & m_shipIDs;
		archive & m_ai;
		archive & m_id;
		archive & m_aiEnabled;
		archive & m_dead;
		archive & m_resources;
		archive & m_chassis;
		archive & m_weapons;
		archive & m_designerShips;
		archive & m_announcementEvents;
		archive & m_name;
		archive & m_availableColonyBuildings;
		archive & m_techs;
		archive & m_currentResearchPoints;
		archive & m_researchPointProduction;
		archive & m_newsEvents;
		archive & m_neverColonized;
		archive & m_numTicksAlive;
	}
	
	Faction() {}

	Star* m_capital = nullptr;
	uint32_t m_capitalID = 0;
	
	sf::Color m_color;
	
	std::vector<Star*> m_ownedSystems;
	std::vector<uint32_t> m_ownedSystemIDs;

	std::vector<Spaceship*> m_ships;
	std::vector<uint32_t> m_shipIDs;

	std::unordered_map<std::string, float> m_resources;
	std::vector<DesignerChassis> m_chassis;
	std::vector<DesignerWeapon> m_weapons;
	std::vector<DesignerShip> m_designerShips;
	std::deque<std::string> m_announcementEvents;
	std::string m_name = "Unnamed";
	std::vector<std::string> m_availableColonyBuildings;
	std::deque<std::string> m_newsEvents;
	std::vector<Tech> m_techs;
	
	Brain m_ai;
	
	int m_id = 0;
	int m_numTicksAlive = 0;
	
	bool m_aiEnabled = false;
	bool m_dead = false;
	bool m_neverColonized = true;

	float m_currentResearchPoints = 0.0f;
	float m_researchPointProduction = 0.0f;
};

