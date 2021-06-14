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

Star::Star(sf::Vector2f pos) {
	init(pos);
}

void Star::init(const sf::Vector2f& pos) {
	m_shape.setRadius(50);
	m_shape.setPosition(pos);
	//m_localViewSprite.setTexture(TextureCache::getTexture("data/art/star1.png"));
	m_localViewSprite.setPosition(pos);
	m_shaderRandomSeed = Random::randFloat(0.0f, 1.0f);

	int starColorRand = Random::randInt(0, 8);
	if (starColorRand <= 3) { // 0, 1, 2, 3
		// Red star
		m_localViewSprite.setColor(sf::Color::Red);
		m_temperature = Random::randFloat(3000.0f, 5000.0f);
	}
	else if (starColorRand <= 6) { // 4, 5, 6
		// Yellow star
		m_localViewSprite.setColor(sf::Color::Yellow);
		m_temperature = Random::randFloat(5000.0f, 10000.0f);

	}
	else if (starColorRand <= 8) { // 7, 8
		if (Random::randBool()) {
			// Blue star
			m_localViewSprite.setColor(sf::Color(0, 255, 255));
			m_temperature = Random::randFloat(10000.0f, 45000.0f);
		}
		else {
			// White star
			m_localViewSprite.setColor(sf::Color::White);
			m_localViewSprite.setScale(sf::Vector2f(0.5f, 0.5f));
			m_temperature = Random::randFloat(15000.0f, 35000.0f);
		}
	}

	generatePlanets();
}

void Star::draw(sf::RenderWindow& window) {
	window.draw(m_shape);
}

void Star::draw(sf::RenderWindow& window, sf::Shader& shader) {
	sf::Vector2i mouseCoords = sf::Mouse::getPosition(window);
	sf::Vector2f mouseCoordsWorld = window.mapPixelToCoords(mouseCoords);

	if (m_multipleFactionsPresent) {
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

		window.draw(m_shape, &shader);
		
		setRadius(getRadius() / 2);
		setPos(oldPos);

		shader.setUniform("radius", getRadius());
	}
	else {
		window.draw(m_shape, &shader);
	}
}

void Star::drawLocalView(sf::RenderWindow& window, EffectsEmitter& emitter, Player& player, float time) {
	bool drawHidden = true;

	if (player.hasFogOfWar()) {
		if (numAllies(player.getFaction()) == 0) {
			drawHidden = false;
			emitter.drawFogOfWar(window);
		}
	}
	
	//window.draw(m_localViewSprite);
	emitter.drawLocalStar(window, m_localViewSprite, time, m_shaderRandomSeed);
	
	for (JumpPoint& j : m_jumpPoints) {
		j.draw(window, emitter);
	}

	for (Planet& planet : m_planets) {
		planet.draw(window, emitter, time);
	}

	//emitter.drawHabitableZone(window, getLocalViewCenter(), m_temperature);

	if (drawHidden) {

		for (std::unique_ptr<Spaceship>& s : m_localShips) {
			s->draw(window, emitter);
		}
		for (std::unique_ptr<Building>& b : m_buildings) {
			b->draw(window);
		}
		for (Projectile& p : m_projectiles) {
			p.draw(window);
		}
		for (Animation& a : m_localViewAnimations) {
			a.draw(window);
		}
		m_particleSystem.drawParticles(window);
	}
}

void Star::setPos(sf::Vector2f pos) {
	m_shape.setPosition(pos);
	m_localViewSprite.setPosition(pos);
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
}

void Star::factionTakeOwnership(Faction* faction, bool spawnClaimUnit) {
	m_allegiance = faction->getID();
	setColor(faction->getColor());
	faction->addOwnedSystem(this);

	if (spawnClaimUnit) {
		createSpaceship(std::make_unique<Spaceship>(Spaceship::SPACESHIP_TYPE::CLAIM_SHIP, getRandomLocalPos(-10000.0f, 10000.0f), this, faction->getID(), faction->getColor()));
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
	sf::IntRect rect = m_localViewSprite.getTextureRect();
	sf::Vector2f pos = getPos();

	pos.x += rect.width / 2.0f;
	pos.y += rect.height / 2.0f;

	return pos;
}

void Star::addProjectile(Projectile proj) {
	m_projectiles.push_back(proj);
}

void Star::handleCollisions() {
	for (Projectile& p : m_projectiles) {
		for (auto& s : m_localShips) {
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
		}
	}
}

void Star::addAnimation(Animation&& anim) {
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

void Star::update(Constellation* constellation) {
	std::vector<int> factions;
	
	for (int i = 0; i < m_localShips.size(); i++) {
		m_localShips[i]->update(this);

		if (m_localShips[i] == nullptr) {
			// It left
			m_localShips.erase(m_localShips.begin() + i);
			i--;
			continue;
		}

		if (factions.size() == 0) {
			factions.push_back(m_localShips[i]->getAllegiance());
		}
		else {
			if (std::find(factions.begin(), factions.end(), m_localShips[i]->getAllegiance()) == factions.end()) {
				factions.push_back(m_localShips[i]->getAllegiance());
			}
		}
		if (m_localShips[i]->isDead()) {
			m_particleSystem.createParticleExplosion(m_localShips[i]->getPos(), m_localShips[i]->getCollider().getOutlineColor(), 10.0f, 100);
			constellation->moveShipToPurgatory(m_localShips[i]);

			m_localShips.erase(m_localShips.begin() + i);
			i--;
		}
	}
	
	for (int i = 0; i < m_buildings.size(); i++) {
		m_buildings[i]->update(this, constellation->getFaction(m_buildings[i]->getAllegiance()));
		if (m_buildings[i]->isDead()) {
			m_particleSystem.createParticleExplosion(m_buildings[i]->getPos(), m_buildings[i]->getCollider().getOutlineColor(), 10.0f, 100);
			
			m_buildings.erase(m_buildings.begin() + i);
			i--;
		}
	}

	if (factions.size() > 1) {
		m_multipleFactionsPresent = true;
	}
	else {
		m_multipleFactionsPresent = false;
	}

	for (Projectile& p : m_projectiles) {
		p.update();
	}
	for (Animation& a : m_localViewAnimations) {
		a.nextFrame();
	}
	for (Planet& planet : m_planets) {
		planet.update();
	}

	m_particleSystem.updateParticles();

	handleCollisions();
	m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(), [](Projectile& p) {return p.isDead(); }), m_projectiles.end());
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

int Star::numAlliedBuildings(int allegiance) const {
	int c = 0;
	for (auto& building : m_buildings) {
		if (building->getAllegiance() == allegiance) {
			c++;
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

bool Star::containsBuildingType(Building::BUILDING_TYPE type, bool allegianceOnly, int allegiance) const {
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
	float latestRadius = Random::randFloat(1000.0f, 10000.0f);

	for (int i = 0; i < numPlanets; i++) {
		float angle = Random::randFloat(0.0f, 2.0f * Math::pi);
		sf::Vector2f pos(latestRadius * std::cos(angle) + getLocalViewCenter().x, latestRadius * std::sin(angle) + getLocalViewCenter().y);

		Planet planet(pos, getLocalViewCenter(), m_temperature * m_localViewSprite.getScale().x);
		m_planets.push_back(planet);

		latestRadius += Random::randFloat(500.0f, 20000.0f);
	}
}