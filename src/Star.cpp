#include "gamepch.h"

#include "Star.h"
#include "Debug.h"
#include "Faction.h"
#include "GameState.h"
#include "TextureCache.h"
#include "EffectsEmitter.h"
#include "Hyperlane.h"
#include "Player.h"
#include "Random.h"
#include "Unit.h"
#include "Constellation.h"
#include "Math.h"
#include "Fonts.h"

Star::Star(sf::Vector2f pos) {
	init(pos);
}

void Star::init(const sf::Vector2f& pos) {
	m_shape.setRadius(50);
	m_shape.setPosition(pos);
	//m_localViewSprite.setTexture(TextureCache::getTexture("data/art/star1.png"));
	m_localViewRect.setPosition(pos);
	m_localViewRect.setSize(sf::Vector2f(2048.0f, 2048.0f));
	m_localViewRect.setScale(16.0, 16.0);
	m_localViewRect.setOrigin(sf::Vector2f(2048.0f/2.0f, 2048/2.0f));
	m_quadtree = Quadtree(getLocalViewCenter(), 1000000);
	m_shaderRandomSeed = Random::randFloat(0.0f, 1.0f);

	int starColorRand = Random::randInt(0, 9);
	if (starColorRand <= 3) { // 0, 1, 2, 3
		// Red star
		m_localViewRect.setFillColor(sf::Color::Red);
		m_temperature = Random::randFloat(3000.0f, 5000.0f);

		if (Random::randInt(0, 4) == 4) {
			m_localViewRect.setScale(32.0f, 32.0f);
		}
	}
	else if (starColorRand <= 6) { // 4, 5, 6
		// Yellow star
		m_localViewRect.setFillColor(sf::Color::Yellow);
		m_temperature = Random::randFloat(5000.0f, 10000.0f);

	}
	else if (starColorRand <= 8) { // 7, 8
		if (Random::randBool()) {
			// Blue star
			m_localViewRect.setFillColor(sf::Color(0, 255, 255));
			m_temperature = Random::randFloat(10000.0f, 45000.0f);
		}
		else {
			// White star
			m_localViewRect.setFillColor(sf::Color::White);
			m_localViewRect.setScale(sf::Vector2f(8.0f, 8.0f));
			m_temperature = Random::randFloat(15000.0f, 35000.0f);
		}
	}
	else if (starColorRand == 9) {
		// Black hole

		m_blackHole = true;
		m_localViewRect.setScale(8.0f, 8.0f);
		m_temperature = Random::randFloat(50000.0f, 100000.0f);
	}

	generatePlanets();
	generateDerelicts();
}

void Star::draw(sf::RenderWindow& window) {
	window.draw(m_shape);
}

void Star::draw(sf::RenderWindow& window, sf::Shader& shader, Constellation& constellation, Player& player) {
	sf::Vector2i mouseCoords = sf::Mouse::getPosition(window);
	sf::Vector2f mouseCoordsWorld = window.mapPixelToCoords(mouseCoords);

	if (m_multipleFactionsPresent && isDiscovered(player.getFaction())) {
		shader.setUniform("flashing", true);
	}
	else {
		shader.setUniform("flashing", false);
	}

	if (isInShapeRadius(mouseCoordsWorld.x, mouseCoordsWorld.y)) {
		sf::Vector2f oldPos = getPos();
		
		setRadius(getRadius() * 2);
		
		sf::Vector2f newPos = getPos();
		newPos.x -= getRadius() / 2;
		newPos.y -= getRadius() / 2;
		
		setPos(newPos);

		shader.setUniform("radius", getRadius());

		if (!isDiscovered(player.getFaction())) {
			drawUndiscovered(window, shader);
		}
		else {
			window.draw(m_shape, &shader);
		}
		
		setRadius(getRadius() / 2);
		setPos(oldPos);

		shader.setUniform("radius", getRadius());
	}
	else {
		if (!isDiscovered(player.getFaction())) {
			drawUndiscovered(window, shader);
		}
		else {
			window.draw(m_shape, &shader);
		}
	}

	// Floaty name text
	sf::Text text;
	text.setFont(Fonts::getFont("data/fonts/consola.ttf"));
	text.setString(m_name);
	text.setOrigin(text.getLocalBounds().width / 2.0f, 0.0f);

	sf::Vector2f pos = getCenter();
	pos.y -= getRadius();

	text.setPosition(pos);
	
	window.draw(text);

	// Draw number of ships text
	if (isDiscovered(player.getFaction())) {
		if (!player.hasFogOfWar() || numAllies(player.getFaction()) > 0) {
			std::unordered_map<int, int> numFactionShips = countNumFactionShips();
			for (auto& ships : numFactionShips) {
				Faction* faction = constellation.getFaction(ships.first);
				if (faction != nullptr) {
					pos.y -= text.getCharacterSize();
					text.setPosition(pos);
					text.setString(std::to_string(ships.second));
					text.setOrigin(text.getLocalBounds().width / 2.0f, 0.0f);
					text.setFillColor(faction->getColor());
					window.draw(text);
				}
			}
		}
	}
}

void Star::drawLocalView(sf::RenderWindow& window, EffectsEmitter& emitter, Player& player, float time) {
	m_drawHidden = true;

	if (player.hasFogOfWar()) {
		if (numAllies(player.getFaction()) == 0) {
			m_drawHidden = false;
			emitter.drawFogOfWar(window);
		}
	}
	
	if (m_blackHole) emitter.drawBlackHole(window, m_localViewRect, time, m_shaderRandomSeed);
	else emitter.drawLocalStar(window, m_localViewRect, time, m_shaderRandomSeed);
	
	for (JumpPoint& j : m_jumpPoints) {
		j.draw(window, emitter);
	}

	//emitter.drawHabitableZone(window, getLocalViewCenter(), m_temperature);

	if (m_drawHidden) {

		for (Planet& planet : m_planets) {
			planet.draw(window, emitter, this, time);
		}
		
		for (std::unique_ptr<Spaceship>& s : m_localShips) {
			s->draw(window, emitter);
		}
		for (std::unique_ptr<Building>& b : m_buildings) {
			b->draw(window, emitter);
		}
		for (Derelict& d : m_derelicts) {
			d.draw(window);
		}
		for (Projectile& p : m_projectiles) {
			p.draw(window);
		}
		for (Animation& a : m_localViewAnimations) {
			a.draw(window);
		}
		m_particleSystem.drawParticles(window);
		//m_quadtree.draw(window);
	}
}

void Star::setPos(sf::Vector2f pos) {
	m_shape.setPosition(pos);
	m_localViewRect.setPosition(pos);
}

float Star::distBetweenStar(Star& s) {
	sf::Vector2f otherPos = s.getPos();
	sf::Vector2f thisPos = getPos();
	return std::sqrt(std::pow(otherPos.x - thisPos.x, 2) + std::pow(otherPos.y - thisPos.y, 2));
}

bool Star::isStarInRadius(Star& s, float radius) const {
	sf::Vector2f otherPos = s.getCenter();
	sf::Vector2f thisPos = getCenter();

	float distX = otherPos.x - thisPos.x;
	float distY = otherPos.y - thisPos.y;

	if (sqrt(std::pow(distX, 2) + std::pow(distY, 2)) <= radius) {
		return true;
	}
	else return false;
}

bool Star::isInShapeRadius(float x, float y) const {
	float radius = m_shape.getRadius();
	sf::Vector2f thisPos = getCenter();

	float distX = x - thisPos.x;
	float distY = y - thisPos.y;

	if (sqrt(std::pow(distX, 2) + std::pow(distY, 2)) <= radius) {
		return true;
	}
	else return false;
}

void Star::connectHyperlane(Hyperlane* lane) {
	m_hyperlanes.push_back(lane);
	m_hyperlaneIDs.push_back(lane->getID());
}

void Star::factionTakeOwnership(Faction* faction, bool spawnClaimUnit) {
	m_allegiance = faction->getID();
	setColor(faction->getColor());
	faction->addOwnedSystem(this);

	if (spawnClaimUnit) {
		createSpaceship(std::make_unique<Spaceship>("CLAIM_SHIP", getRandomLocalPos(-10000.0f, 10000.0f), this, faction->getID(), faction->getColor()));
	}
}

sf::Vector2f Star::getCenter() const {
	sf::Vector2f pos = getPos();
	pos.x += getRadius();
	pos.y += getRadius();
	return pos;
}

void Star::clicked(sf::Event ev, GameState& state) {
	state.changeToLocalView(this);
}

void Star::setupJumpPoints() {
	std::vector<Hyperlane*> outgoingHyperlanes;
	std::vector<Hyperlane*> incomingHyperlanes;
	const int minDist = 5000;
	const int maxDist = 20000;

	for (Hyperlane* h : m_hyperlanes) {
		if (h->getEndStar() == this) {
			incomingHyperlanes.push_back(h);
		}
		else {
			outgoingHyperlanes.push_back(h);
		}
	}

	for (Hyperlane* h : outgoingHyperlanes) {
		float randDist = rand() % (maxDist - minDist) + minDist;
		float angle = h->getAngleRadians();
		sf::Vector2f thisPos = m_shape.getPosition();

		m_jumpPoints.emplace_back(sf::Vector2f((randDist * std::cos(angle)) + thisPos.x, (randDist * std::sin(angle)) + thisPos.y), angle, h, true);
	}

	for (Hyperlane* h : incomingHyperlanes) {
		float randDist = rand() % (maxDist - minDist) + minDist;
		float angle = h->getAngleRadians() + 3.14159;
		sf::Vector2f thisPos = m_shape.getPosition();

		m_jumpPoints.emplace_back(sf::Vector2f((randDist * std::cos(angle)) + thisPos.x, (randDist * std::sin(angle)) + thisPos.y), angle, h, false);
	}
}

sf::Vector2f Star::getLocalViewCenter() const {
	/*sf::IntRect rect = m_localViewSprite.getTextureRect();
	sf::Vector2f pos = getPos();

	pos.x += rect.width / 2.0f;
	pos.y += rect.height / 2.0f;

	return pos;*/
	return m_localViewRect.getPosition();
}

void Star::addProjectile(Projectile proj) {
	m_projectiles.push_back(proj);
}

void Star::handleCollisions() {
	// Fill quadtree
	m_quadtree = Quadtree(getLocalViewCenter(), 100000);

	for (auto& ship : m_localShips) {
		m_quadtree.insert(ship.get());
	}
	for (auto& building : m_buildings) {
		m_quadtree.insert(building.get());
	}
	
	for (Projectile& p : m_projectiles) {
		std::vector<Unit*> nearUnits;
		m_quadtree.retrieve(nearUnits, p.getCollider());

		for (Unit* unit : nearUnits) {
			if (p.isCollidingWith(unit->getCollider()) && p.getAllegiance() != unit->getAllegiance()) {
				unit->takeDamage(p.getDamage());
				if (p.diesOnCollision()) p.kill();
				m_particleSystem.createParticle(
					ParticleSystem::Particle{ 1000, Random::randVec(-10.0f, 10.0f) }, p.getPos(), unit->getCollider().getColor()
				);
			}
		}

		//DEBUG_PRINT("Comparisons: " << nearUnits.size());

		/*for (auto& s : m_localShips) {
			if (p.isCollidingWith(s->getCollider()) && p.getAllegiance() != s->getAllegiance()) {
				s->takeDamage(p.getDamage());
				p.kill();
				m_particleSystem.createParticle(
					ParticleSystem::Particle{ 1000, Random::randVec(-10.0f, 10.0f) }, p.getPos(), s->getCollider().getOutlineColor()
				);
			}
		}
		for (auto& b : m_buildings) {
			if (p.isCollidingWith(b->getCollider()) && p.getAllegiance() != b->getAllegiance()) {
				if (!b->isBuilt()) {
					b->kill();
					p.kill();
				}
				else {
					b->takeDamage(p.getDamage());
					p.kill();
				}
				m_particleSystem.createParticle(
					ParticleSystem::Particle{ 1000, Random::randVec(-10.0f, 10.0f) }, p.getPos(), b->getCollider().getOutlineColor()
				);
			}
		}*/

		//DEBUG_PRINT("Comparisons: " << m_localShips.size() + m_buildings.size());
	}
}

void Star::addAnimation(const Animation& anim) {
	if (m_localViewActive) {
		m_localViewAnimations.push_back(anim);
	}
}

void Star::cleanUpAnimations() {
	for (int i = 0; i < m_localViewAnimations.size(); i++) {
		if (m_localViewAnimations[i].isDone()) {
			m_localViewAnimations.erase(m_localViewAnimations.begin() + i);
			i--;
		}
	}
}

void Star::update(Constellation* constellation, const Player& player) {
	std::set<int> factions;
	
	// Update spaceships
	for (int i = 0; i < m_localShips.size(); i++) {
		m_localShips[i]->updateMods(this, constellation->getFaction(m_localShips[i]->getAllegiance()));
		m_localShips[i]->update(this);

		if (m_localShips[i] == nullptr) {
			// It left
			m_localShips.erase(m_localShips.begin() + i);
			i--;
			continue;
		}
		if (!m_localShips[i]->isCivilian()) {
			factions.insert(m_localShips[i]->getAllegiance());
		}
		if (m_localShips[i]->isDead()) {
			m_particleSystem.createParticleExplosion(m_localShips[i]->getPos(), m_localShips[i]->getCollider().getColor(), 10.0f, 100);
			m_localShips[i]->onDeath(this);
			constellation->moveShipToPurgatory(m_localShips[i]);

			m_localShips.erase(m_localShips.begin() + i);
			i--;
		}
	}
	
	// Update buildings
	for (int i = 0; i < m_buildings.size(); i++) {
		m_buildings[i]->updateMods(this, constellation->getFaction(m_buildings[i]->getAllegiance()));
		m_buildings[i]->update(this);
		if (m_buildings[i]->isDead()) {
			m_particleSystem.createParticleExplosion(m_buildings[i]->getPos(), m_buildings[i]->getCollider().getColor(), 10.0f, 100);
			m_buildings[i]->onDeath(this);

			m_buildings.erase(m_buildings.begin() + i);
			i--;
		}
	}

	// Discover system for factions
	for (int i : factions) {
		m_factionsDiscovered.insert(i);
	}

	// For the flashy thing
	if (factions.size() > 1) {
		m_multipleFactionsPresent = true;
	}
	else {
		m_multipleFactionsPresent = false;
	}

	// Update projectiles
	for (int i = 0; i < m_projectiles.size(); i++) {
		m_projectiles[i].update(this);

		if (m_projectiles[i].isDead()) {
			m_projectiles[i].onDeath(this);
			m_projectiles.erase(m_projectiles.begin() + i);
			i--;
		}
	}
	for (Animation& a : m_localViewAnimations) {
		a.nextFrame();
	}
	for (Planet& planet : m_planets) {
		planet.update(this, constellation->getFaction(planet.getColony().getAllegiance()));
	}
	for (int i = 0; i < m_derelicts.size(); i++) {
		m_derelicts[i].update(this, constellation->getFactions());

		if (m_derelicts[i].isDead()) {
			m_derelicts.erase(m_derelicts.begin() + i);
			i--;
		}
	}

	m_particleSystem.updateParticles();

	handleCollisions();
	cleanUpAnimations();
}

void Star::destroyAllShips() {
	for (auto& s : m_localShips) {
		s->kill();
	}
}

std::vector<Star*> Star::getConnectedStars() {
	std::vector<Star*> stars;
	for (JumpPoint& j : m_jumpPoints) {
		stars.push_back(j.getConnectedOtherStar());
	}
	return stars;
}

int Star::numAlliedShips(int allegiance) const {
	int count = 0;
	for (auto& s : m_localShips) {
		if (s->getAllegiance() == allegiance) {
			count++;
		}
	}
	return count;
}

sf::Vector2f Star::getRandomLocalPos(float min, float max) const {
	return getLocalViewCenter() + Random::randVec(min, max);
}

Unit* Star::getUnitByID(unsigned int id) {
	Spaceship* ship = getShipByID(id);
	if (ship != nullptr) return ship;
	
	Building* building = getBuildingByID(id);
	if (building != nullptr) return building;
	
	return nullptr;
}

Spaceship* Star::getShipByID(unsigned int id) {
	for (auto& ship : m_localShips) {
		if (ship->getID() == id) {
			return ship.get();
		}
	}
	return nullptr;
}

Building* Star::getBuildingByID(unsigned int id) {
	for (auto& building : m_buildings) {
		if (building->getID() == id) {
			return building.get();
		}
	}
	return nullptr;
}

JumpPoint* Star::getJumpPointByID(unsigned int id) {
	for (JumpPoint& j : m_jumpPoints) {
		if (j.getID() == id) {
			return &j;
		}
	}
	return nullptr;
}

Spaceship* Star::createSpaceship(std::unique_ptr<Spaceship>&& ship) {
	m_localShips.push_back(std::move(ship));
	return m_localShips.back().get();
}

Spaceship* Star::createSpaceship(std::unique_ptr<Spaceship>& ship) {
	m_localShips.push_back(std::move(ship));
	return m_localShips.back().get();
}

void Star::moveShipToOtherStar(Spaceship* ship, Star* other) {
	for (int i = 0; i < m_localShips.size(); i++) {
		if (m_localShips[i].get() == ship) {
			other->m_localShips.push_back(std::move(m_localShips[i]));
			return;
		}
	}
}

int Star::numAlliedBuildings(int allegiance, const std::string& type) const {
	int c = 0;
	for (auto& building : m_buildings) {
		if (building->getAllegiance() == allegiance) {
			if (type != "" && building->getType() == type) c++;
			else if (type == "") c++;
		}
	}
	return c;
}

Building* Star::createBuilding(std::unique_ptr<Building>&& building) {
	m_buildings.push_back(std::move(building));
	return m_buildings.back().get();
}

Building* Star::createBuilding(std::unique_ptr<Building>& building) {
	m_buildings.push_back(std::move(building));
	return m_buildings.back().get();
}

int Star::numAllies(int allegiance) const {
	return numAlliedShips(allegiance) + numAlliedBuildings(allegiance);
}

bool Star::containsBuildingType(const std::string& type, bool allegianceOnly, int allegiance) const {
	for (auto& building : m_buildings) {
		if (allegianceOnly) {
			if (building->getType() == type && building->getAllegiance() == allegiance) {
				return true;
			}
		}
		else {
			if (building->getType() == type) {
				return true;
			}
		}
	}
	return false;
}

void Star::generatePlanets() {
	int numPlanets = Random::randInt(0, 15);
	float starRadius = m_localViewRect.getSize().x / 2.0f * m_localViewRect.getScale().x;
	float latestRadius = Random::randFloat(1000.0f + starRadius, 10000.0f + starRadius);

	for (int i = 0; i < numPlanets; i++) {
		float angle = Random::randFloat(0.0f, 2.0f * Math::pi);
		sf::Vector2f pos(latestRadius * std::cos(angle) + getLocalViewCenter().x, latestRadius * std::sin(angle) + getLocalViewCenter().y);

		Planet planet(pos, getLocalViewCenter(), m_temperature);
		m_planets.push_back(planet);

		latestRadius += Random::randFloat(500.0f, 20000.0f);
	}
}

Planet* Star::getPlanetByID(unsigned int id) {
	for (Planet& planet : m_planets) {
		if (planet.getID() == id) {
			return &planet;
		}
	}
	return nullptr;
}

Planet& Star::getMostHabitablePlanet() {
	float highest = 0.0f;
	int index = 0;
	for (int i = 0; i < m_planets.size(); i++) {
		if (m_planets[i].getHabitability() > highest) {
			highest = m_planets[i].getHabitability();
			index = i;
		}
	}
	return m_planets[index];
}

void Star::drawUndiscovered(sf::RenderWindow& window, sf::Shader& shader) {
	sf::Color oldColor;
	oldColor = m_shape.getFillColor();
	m_shape.setFillColor(sf::Color(166, 166, 166));
	window.draw(m_shape, &shader);
	m_shape.setFillColor(oldColor);
}

void Star::generateDerelicts() {
	if (Random::randBool()) {
		sf::Vector2f pos = Random::randVec(-10000.0f, 10000.0f);
		Derelict d(pos);
		m_derelicts.push_back(d);
	}
}

std::vector<Spaceship*> Star::getAllShipsOfAllegiance(int allegiance) {
	std::vector<Spaceship*> ships;
	for (auto& ship : m_localShips) {
		if (ship->getAllegiance() == allegiance) {
			ships.push_back(ship.get());
		}
	}
	return ships;
}

Hyperlane* Star::getHyperlaneByID(uint32_t id) {
	for (Hyperlane* hyperlane : m_hyperlanes) {
		if (hyperlane->getID() == id) {
			return hyperlane;
		}
	}
	return nullptr;
}

void Star::reinitAfterLoad(Constellation* constellation) {
	for (uint32_t id : m_hyperlaneIDs) {
		m_hyperlanes.push_back(constellation->getHyperlaneByID(id));
	}

	for (JumpPoint& jp : m_jumpPoints) {
		jp.reinitAfterLoad(this);
	}

	for (auto& spaceship : m_localShips) {
		spaceship->reinitAfterLoad(this);
		spaceship->reinitOrdersAfterLoad(constellation);
	}

	for (auto& building : m_buildings) {
		building->reinitAfterLoad(this);
	}
}

Planet* Star::getMostHabitablePlanet(int allegiance) {
	Planet* mostHabitable = nullptr;
	for (Planet& planet : m_planets) {
		if (mostHabitable != nullptr) {
			if (planet.getColony().getAllegiance() == allegiance && planet.getHabitability() > mostHabitable->getHabitability()) {
				mostHabitable = &planet;
			}
		}
		else {
			mostHabitable = &planet;
		}
	}
	return mostHabitable;
}

void Star::setDiscovered(bool isDiscovered, int allegiance) {
	if (isDiscovered) {
		m_factionsDiscovered.insert(allegiance);
	}
	else {
		if (m_factionsDiscovered.count(allegiance)) {
			m_factionsDiscovered.erase(allegiance);
		}
	}
}

std::vector<Planet*> Star::getEnemyPlanets(int allegiance) {
	std::vector<Planet*> planets;
	for (Planet& planet : getPlanets()) {
		if (planet.getColony().getAllegiance() != -1 && planet.getColony().getAllegiance() != allegiance) {
			planets.push_back(&planet);
		}
	}
	return planets;
}

std::vector<Spaceship*> Star::getEnemyCombatShips(int allegiance) {
	std::vector<Spaceship*> ships;
	for (auto& ship : m_localShips) {
		if (ship->getAllegiance() != allegiance && !ship->isCivilian()) {
			ships.push_back(ship.get());
		}
	}
	return ships;
}

std::vector<Building*> Star::getBuildingsOfType(const std::string& type) {
	std::vector<Building*> buildings;
	for (auto& building : m_buildings) {
		if (building->getType() == type) {
			buildings.push_back(building.get());
		}
	}
	return buildings;
}

std::unordered_map<int, int> Star::countNumFactionShips() {
	std::unordered_map<int, int> numFactionShips;
	for (auto& ship : m_localShips) {
		if (numFactionShips.count(ship->getAllegiance()) > 0) {
			numFactionShips[ship->getAllegiance()]++;
		}
		else {
			numFactionShips[ship->getAllegiance()] = 1;
		}
	}
	return numFactionShips;
}