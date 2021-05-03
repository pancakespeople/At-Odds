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

Star::Star(sf::Vector2f pos) {
	init(pos);
}

Star::Star() {
	init(sf::Vector2f(0.0f, 0.0f));
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
	}
	else if (starColorRand <= 6) { // 4, 5, 6
		// Yellow star
		m_localViewSprite.setColor(sf::Color::Yellow);

	}
	else if (starColorRand <= 8) { // 7, 8
		if (Random::randBool()) {
			// Blue star
			m_localViewSprite.setColor(sf::Color(0, 255, 255));
		}
		else {
			// White star
			m_localViewSprite.setColor(sf::Color::White);
			m_localViewSprite.setScale(sf::Vector2f(0.5f, 0.5f));
		}
	}
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
		if (numAlliedShips(player.getFaction()) == 0) {
			drawHidden = false;
			emitter.drawFogOfWar(window);
		}
	}
	
	//window.draw(m_localViewSprite);
	emitter.drawLocalStar(window, m_localViewSprite, time, m_shaderRandomSeed);
	for (JumpPoint& j : m_jumpPoints) {
		j.draw(window, emitter);
	}

	if (drawHidden) {

		for (Spaceship* s : m_localShips) {
			s->draw(window, emitter);
		}
		for (Building& b : m_buildings) {
			b.draw(window);
		}
		for (Projectile& p : m_projectiles) {
			p.draw(window);
		}
		for (Animation& a : m_localViewAnimations) {
			a.draw(window);
		}


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

bool Star::isStarInRadius(Star& s, float radius) {
	sf::Vector2f otherPos = s.getCenter();
	sf::Vector2f thisPos = getCenter();

	float distX = otherPos.x - thisPos.x;
	float distY = otherPos.y - thisPos.y;

	if (sqrt(std::pow(distX, 2) + std::pow(distY, 2)) <= radius) {
		return true;
	}
	else return false;
}

bool Star::isInShapeRadius(float x, float y) {
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
		faction->createShip(Spaceship(Spaceship::SPACESHIP_TYPE::CLAIM_SHIP, getRandomLocalPos(-10000.0f, 10000.0f), this, faction->getID(), faction->getColor()));
	}
}

sf::Vector2f Star::getCenter() {
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

sf::Vector2f Star::getLocalViewCenter() {
	sf::IntRect rect = m_localViewSprite.getTextureRect();
	sf::Vector2f pos = getPos();

	pos.x += rect.width / 2.0f;
	pos.y += rect.height / 2.0f;

	return pos;
}

void Star::addSpaceship(Spaceship* ship) {
	m_localShips.push_back(ship);
}

void Star::removeSpaceship(Spaceship* ship) {
	m_localShips.erase(std::remove(m_localShips.begin(), m_localShips.end(), ship), m_localShips.end());
}

void Star::addProjectile(Projectile proj) {
	m_projectiles.push_back(proj);
}

void Star::handleCollisions() {
	for (Projectile& p : m_projectiles) {
		for (Spaceship* s : m_localShips) {
			if (p.isCollidingWith(s->getCollider()) && p.getAllegiance() != s->getAllegiance()) {
				s->takeDamage(p.getDamage());
				p.kill();
			}
		}
		for (Building& b : m_buildings) {
			if (p.isCollidingWith(b.getCollider()) && p.getAllegiance() != b.getAllegiance()) {
				b.takeDamage(p.getDamage());
				p.kill();
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

void Star::update() {
	std::vector<int> factions;
	
	for (Spaceship* s : m_localShips) {
		s->update();
		if (factions.size() == 0) {
			factions.push_back(s->getAllegiance());
		}
		else {
			if (std::find(factions.begin(), factions.end(), s->getAllegiance()) == factions.end()) {
				factions.push_back(s->getAllegiance());
			}
		}
	}
	
	for (int i = 0; i < m_buildings.size(); i++) {
		m_buildings[i].update();
		if (m_buildings[i].isDead()) {
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
	
	handleCollisions();
	m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(), [](Projectile& p) {return p.isDead(); }), m_projectiles.end());
	cleanUpAnimations();
}

void Star::destroyAllShips() {
	for (Spaceship* s : m_localShips) {
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

int Star::numAlliedShips(int allegiance) {
	int count = 0;
	for (Spaceship* s : m_localShips) {
		if (s->getAllegiance() == allegiance) {
			count++;
		}
	}
	return count;
}

sf::Vector2f Star::getRandomLocalPos(float min, float max) {
	return getLocalViewCenter() + Random::randVec(min, max);
}