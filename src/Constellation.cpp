#include "gamepch.h"

#include "Constellation.h"
#include "Debug.h"
#include "GameState.h"
#include "Faction.h"
#include "Random.h"
#include "Math.h"
#include "ext/delaunator.hpp"

Constellation::Constellation() {
    m_availableFactionColors = {
        {sf::Color(255, 0, 0), "Red"},
        {sf::Color(0, 255, 0), "Lime"},
        {sf::Color(0, 0, 255), "Blue"},
        {sf::Color(255, 255, 0), "Yellow"},
        {sf::Color(0, 255, 255), "Cyan"},
        {sf::Color(255, 0, 255), "Magenta"},
        {sf::Color(128, 0, 0), "Maroon"},
        {sf::Color(128,128,0), "Olive"},
        {sf::Color(0,128,0), "Green"},
        {sf::Color(128,0,128), "Purple"},
        {sf::Color(0,128,128), "Teal"},
        {sf::Color(0,0,128), "Navy"},
        {sf::Color(255,128,0), "Orange"},
        {sf::Color(102,51,0), "Brown"},
        {sf::Color(102,178,255), "Light Blue"},
        {sf::Color(153,255,51), "Light Green"},
    };
}

void Constellation::recursiveConstellation(std::unique_ptr<Star>& root, int maxSize) {
    const int maxDistStar = 500;
    const int minDistStar = 100;
    int numConnections = rand() % 5;
    int initialX = root->getPos().x;
    int initialY = root->getPos().y;
    int starIdx = m_stars.size() - 1;

    for (int i = 0; i < numConnections; i++) {
        float dist = rand() % (maxDistStar - minDistStar) + minDistStar;
        float randAngle = (rand() % 360) * (3.14159 / 180);

        sf::Vector2f nstarPos((std::cos(randAngle) * dist) + initialX, (std::sin(randAngle) * dist) + initialY);

        // Try to keep some distance between stars
        int loops = 0; // Prevent infinite loop
        while (findClosestStarDistance(nstarPos) < minDistStar && loops < 10) {
            dist = rand() % (maxDistStar - minDistStar) + minDistStar;
            randAngle = (rand() % 360) * (3.14159 / 180);
            nstarPos = sf::Vector2f((std::cos(randAngle) * dist) + initialX, (std::sin(randAngle) * dist) + initialY);
            loops++;
        }

        m_stars.push_back(std::move(std::make_unique<Star>(Star(nstarPos))));
        m_hyperlanes.push_back(std::move(std::make_unique<Hyperlane>(m_stars[starIdx].get(), m_stars.back().get())));

        recursiveConstellation(m_stars.back(), maxSize);

        if (m_stars.size() >= maxSize) return;
    }
}

void Constellation::generateRecursiveConstellation(int sizeWidth, int sizeHeight, int numStars) {
    int initialX = rand() % sizeWidth;
    int initialY = rand() % sizeHeight;
    const int maxDistStar = 500;
    const int minDistStar = 100;
    int numConnections = (rand() % 4) + 1;

    DEBUG_PRINT("Number of connections for root: " << numConnections);

    m_stars.reserve(numStars);
    m_hyperlanes.reserve(numStars);

    //Star root(sf::Vector2f(initialX, initialY));
    m_stars.push_back(std::move(std::make_unique<Star>(Star(sf::Vector2f(initialX, initialY)))));

    for (int i = 0; i < numConnections; i++) {
        float dist = rand() % (maxDistStar - minDistStar) + minDistStar;
        float randAngle = (rand() % 360) * (3.14159 / 180);

        sf::Vector2f nstarPos((std::cos(randAngle) * dist) + initialX, (std::sin(randAngle) * dist) + initialY);

        DEBUG_PRINT(nstarPos.x << " " << nstarPos.y);

        //Star newStar(nstarPos);
        //stars.push_back(newStar);
        m_stars.push_back(std::move(std::make_unique<Star>(Star(nstarPos))));

        //Hyperlane connector(stars[0], stars[stars.size() - 1]);
        //hyperlanes.push_back(connector);
        m_hyperlanes.push_back(std::move(std::make_unique<Hyperlane>(m_stars[0].get(), m_stars.back().get())));

        recursiveConstellation(m_stars.back(), numStars - numConnections);
    }

    // Choose random star to branch off of if not enough stars
    while (m_stars.size() < numStars) {
        std::unique_ptr<Star>& randomStar = m_stars[rand() % (m_stars.size() - 1)];
        recursiveConstellation(randomStar, numStars);
    }

    DEBUG_PRINT("Generation finished with " << m_stars.size() << " stars");
}

void Constellation::generateModernMegaRobustFinalConstellation(int sizeWidth, int sizeHeight, int numStars) {
    sf::Vector2f initialPos = sf::Vector2f(Random::randFloat(0.0f, sizeWidth), Random::randFloat(0.0f, sizeHeight));
    const int minStarDist = 250;
    const int maxStarDist = 750;

    m_stars.push_back(std::make_unique<Star>(initialPos));
    int starsToMake = numStars - 1;
    int index = 0;

    while (starsToMake > 0) {
        int numConnections = 1;
        if (Random::randFloat(0.0f, 1.0f) < 0.25f) {
            numConnections = Random::randInt(1, 5);
        }

        for (int i = 0; i < numConnections && starsToMake > 0; i++) {
            float dist = Random::randFloat(minStarDist, maxStarDist);
            float angle = Random::randFloat(0, Math::pi * 2.0f);
            sf::Vector2f pos(std::cos(angle) * dist + m_stars[index]->getPos().x, std::sin(angle) * dist + m_stars[index]->getPos().y);

            // Try to keep some distance between stars
            int loops = 0; // Prevent infinite loop
            while (findClosestStarDistance(pos) < minStarDist && loops < 10) {
                dist = Random::randFloat(minStarDist, maxStarDist);
                angle = Random::randFloat(0, Math::pi * 2.0f);
                pos = sf::Vector2f(std::cos(angle) * dist + m_stars[index]->getPos().x, std::sin(angle) * dist + m_stars[index]->getPos().y);
                loops++;
            }

            m_stars.push_back(std::make_unique<Star>(pos));
            m_hyperlanes.push_back(std::make_unique<Hyperlane>(m_stars[index].get(), m_stars[m_stars.size() - 1].get()));

            starsToMake--;
        }

        index++;
    }

    DEBUG_PRINT("Modern mega robust final constellation generation finished with " << m_stars.size() << " stars");
}

void Constellation::generateTheReallyFinalRobustConstellationIMeanItReally(int sizeWidth, int sizeHeight, int numStars) {
    // Generate random points
    std::vector<double> coords;
    const int minStarDist = 250;
    const int maxStarDist = 750;
    
    coords.push_back(Random::randFloat(0, sizeWidth));
    coords.push_back(Random::randFloat(0, sizeHeight));
    
    for (std::size_t i = 1; i < numStars; i++) {
        float dist = Random::randFloat(minStarDist, maxStarDist);
        float angle = Random::randFloat(0, Math::pi * 2.0f);
        sf::Vector2f pos(std::cos(angle) * dist + coords[i * 2 - 2], std::sin(angle) * dist + coords[i * 2 - 1]);
        
        // Try to keep some distance between stars
        int loops = 0; // Prevent infinite loop
        while (closestStarDistanceCoords(pos, coords) < minStarDist && loops < 10) {
            dist = Random::randFloat(minStarDist, maxStarDist);
            angle = Random::randFloat(0, Math::pi * 2.0f);
            pos = sf::Vector2f(std::cos(angle) * dist + coords[i * 2 - 2], std::sin(angle) * dist + coords[i * 2 - 1]);
            loops++;
        }

        coords.push_back(pos.x);
        coords.push_back(pos.y);
    }

    delaunator::Delaunator d(coords);
    std::map<std::pair<float, float>, Star*> stars;

    for (std::size_t i = 0; i < coords.size(); i += 2) {
        sf::Vector2f pos(coords[i], coords[i + 1]);
        m_stars.push_back(std::make_unique<Star>(pos));
        stars[std::pair<float, float>(pos.x, pos.y)] = m_stars.back().get();
    }

    for (std::size_t i = 0; i < d.triangles.size(); i += 3) {
        sf::Vector2f pos1(d.coords[2 * d.triangles[i]], d.coords[2 * d.triangles[i] + 1]);
        sf::Vector2f pos2(d.coords[2 * d.triangles[i + 1]], d.coords[2 * d.triangles[i + 1] + 1]);
        sf::Vector2f pos3(d.coords[2 * d.triangles[i + 2]], d.coords[2 * d.triangles[i + 2] + 1]);

        Star* star1 = stars[std::pair<float, float>(pos1.x, pos1.y)];
        Star* star2 = stars[std::pair<float, float>(pos2.x, pos2.y)];
        Star* star3 = stars[std::pair<float, float>(pos3.x, pos3.y)];

        float side1Length = Math::distance(star1->getPos(), star2->getPos());
        float side2Length = Math::distance(star2->getPos(), star3->getPos());
        //float side3Length = Math::distance(star3->getPos(), star1->getPos());
        float longestLength = std::max(side1Length, side2Length);

        if (!star1->hasHyperlaneConnectionTo(star2) && side1Length != longestLength) m_hyperlanes.push_back(std::make_unique<Hyperlane>(star1, star2));
        if (!star2->hasHyperlaneConnectionTo(star3) && side2Length != longestLength) m_hyperlanes.push_back(std::make_unique<Hyperlane>(star2, star3));
        //if (!star3->hasHyperlaneConnectionTo(star1) && side3Length != longestLength) m_hyperlanes.push_back(std::make_unique<Hyperlane>(star3, star1));
    }
}

void Constellation::draw(sf::RenderWindow& window, EffectsEmitter& emitter, Player& player) {

    // Draw empire borders
    for (Faction& faction : m_factions) {
        std::vector<sf::Glsl::Vec3> points;

        for (auto& star : m_stars) {
            sf::Vector2f pos = star->getCenter() - m_border.getPosition();
            if (star->getAllegiance() == faction.getID() && star->isDiscovered(player.getFaction())) points.push_back(sf::Glsl::Vec3(pos.x, pos.y, 1.0f));
            else points.push_back(sf::Glsl::Vec3(pos.x, pos.y, 0.0f));
        }
        
        if (points.size() > 0) emitter.drawBorders(window, m_border, points, faction.getColor());   
    }
    
    for (std::unique_ptr<Hyperlane>& h : m_hyperlanes) {
        h->draw(window, player.getFaction());
    }
    for (std::unique_ptr<Star>& s : m_stars) {
        s->draw(window, emitter, *this, player);
    }
}

void Constellation::generateRandomHyperlanes(int size, int numStars) {
    for (int i = 0; i < numStars; i++) {
        sf::Vector2f randPos(rand() % size, rand() % size);
        m_stars.push_back(std::move(std::make_unique<Star>(Star(randPos))));
    }

    for (std::unique_ptr<Star>& s : m_stars) {
        int otherStarIndex = rand() % (m_stars.size() - 1);
        while (&m_stars[otherStarIndex] == &s) {
            otherStarIndex = rand() % (m_stars.size() - 1);
        }
        m_hyperlanes.push_back(std::move(std::make_unique<Hyperlane>(Hyperlane(s.get(), m_stars[otherStarIndex].get()))));
    }
}
//
//void Constellation::generateRobustHyperlanes(int size, int numStars) {
//    for (int i = 0; i < numStars; i++) {
//        sf::Vector2f randPos(rand() % size, rand() % size);
//        Star nstar(randPos);
//        m_stars.push_back(std::make_unique<Star>(nstar));
//    }
//
//    // Get the average distance between each closest star then average that
//    float sumDistClosestStars = 0.0f;
//    float avgDistClosestStars;
//
//    for (int i = 0; i < m_stars.size(); i++) {
//        float distClosestStar = 0.0f;
//
//        if (i + 1 == m_stars.size()) {
//            distClosestStar = m_stars[i]->distBetweenStar(*m_stars[0]);
//        }
//        else {
//            distClosestStar = m_stars[i]->distBetweenStar(*m_stars[i + 1]);
//        }
//
//        // Find distance for closest star
//        for (int j = 0; j < m_stars.size(); j++) {
//            if (m_stars[j] != m_stars[i]) {
//                if (m_stars[i]->distBetweenStar(*m_stars[j]) < distClosestStar) {
//                    distClosestStar = m_stars[i]->distBetweenStar(*m_stars[j]);
//                }
//            }
//        }
//
//        //DEBUG_PRINT(distClosestStar);
//        sumDistClosestStars += distClosestStar;
//    }
//    avgDistClosestStars = sumDistClosestStars / m_stars.size();
//
//    DEBUG_PRINT("Average distance between stars: " << avgDistClosestStars);
//
//    for (int i = 0; i < m_stars.size(); i++) {
//        std::vector<Star*> closeStars;
//        for (int j = 0; j < m_stars.size(); j++) {
//            if (m_stars[i]->isStarInRadius(*m_stars[j], avgDistClosestStars * 4) && m_stars[i] != m_stars[j]) {
//                closeStars.push_back(m_stars[j].get());
//            }
//        }
//
//        if (closeStars.size() > 0) {
//            int randIndex = rand() % closeStars.size();
//            m_hyperlanes.push_back(std::make_unique<Hyperlane>(Hyperlane(m_stars[i].get(), closeStars[randIndex])));
//        }
//    }
//}
//
void Constellation::onEvent(sf::Event ev, sf::RenderWindow& window, GameState& state) {
    static sf::Vector2f lastMousePressPos;

    if (ev.type == sf::Event::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mouseCoords = sf::Mouse::getPosition(window);
        sf::Vector2f mouseCoordsWorld = window.mapPixelToCoords(mouseCoords);

        for (auto& star : m_stars) {
            if (star->isInShapeRadius(mouseCoordsWorld.x, mouseCoordsWorld.y) &&
                star->isInShapeRadius(lastMousePressPos.x, lastMousePressPos.y)) {
                star->clicked(ev, state);
            }
        }
    }
    else if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
        lastMousePressPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    }
}

float Constellation::distBetweenVecs(const sf::Vector2f& a, const sf::Vector2f& b) {
    return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2));
}

float Constellation::findClosestStarDistance(sf::Vector2f& targetPos) {
    float closest = distBetweenVecs(targetPos, m_stars[0]->getPos());
    for (auto& s : m_stars) {
        float dist = distBetweenVecs(targetPos, s->getPos());
        if (dist < closest) {
            closest = dist;
        }
    }
    return closest;
}

void Constellation::setupStars() {
    sf::Vector2f leftmost = m_stars[0]->getPos();
    sf::Vector2f rightmost = m_stars[0]->getPos();
    sf::Vector2f bottommost = m_stars[0]->getPos();
    sf::Vector2f topmost = m_stars[0]->getPos();
    
    for (std::unique_ptr<Star>& s : m_stars) {
        s->setupJumpPoints();
        s->setName(m_nameGenerator.generateName());

        if (s->getPos().x < leftmost.x) leftmost = s->getCenter();
        if (s->getPos().x > rightmost.x) rightmost = s->getCenter();
        if (s->getPos().y > bottommost.y) bottommost = s->getCenter();
        if (s->getPos().y < topmost.y) topmost = s->getCenter();
    }

    m_border.setPosition(leftmost.x - 1000.0f, topmost.y - 1000.0f);
    m_border.setSize(sf::Vector2f(rightmost.x - leftmost.x + 2000.0f, bottommost.y - topmost.y + 2000.0f));
}

void Constellation::generateFactions(int numFactions) {
    for (int i = 0; i < numFactions; i++) {
        Faction newFaction(i);

        if (m_availableFactionColors.size() > 0) {
            int rndIndex = Random::randInt(0, m_availableFactionColors.size() - 1);
            std::pair<sf::Color, std::string>& color = m_availableFactionColors[rndIndex];
            newFaction.setColor(color.first);
            newFaction.setName(color.second);
            m_availableFactionColors.erase(m_availableFactionColors.begin() + rndIndex);
        }

        newFaction.spawnAtRandomStar(this);
        m_factions.push_back(std::move(newFaction));
    }
}

void Constellation::update(const Player& player) {
    updatePirates();
    for (auto& f : m_factions) {
        f.update();
    }
    for (std::unique_ptr<Star>& s : m_stars) {
        s->update(this, player);
    }
    cleanUpDeadShips();

    m_numUpdates++;
}

void Constellation::cleanUpDeadShips() {
    for (int i = 0; i < m_shipPurgatory.size(); i++) {
        if (m_shipPurgatory[i].timer == 0) {
            m_shipPurgatory.erase(m_shipPurgatory.begin() + i);
            i--;
        }
        else {
            m_shipPurgatory[i].timer--;
        }
    }
}

void Constellation::generateNeutralSquatters() {
    // Generate neutral/hostile ships
    
    for (std::unique_ptr<Star>& star : m_stars) {
        if (star->getAllegiance() == -1) {

            int numShips = Random::randInt(0, 10);
            for (int i = 0; i < numShips; i++) {
                sf::Vector2f pos = star->getRandomLocalPos(-10000, 10000);
                Spaceship* ship = star->createSpaceship("FRIGATE_1", pos, -1, sf::Color(175, 175, 175));
                if (Random::randBool()) {
                    ship->addWeapon(Weapon("LASER_GUN"));
                }
                else {
                    ship->addWeapon(Weapon("MACHINE_GUN"));
                }
            }
        }
    }
}

void Constellation::moveShipToPurgatory(std::unique_ptr<Spaceship>& ship) {
    PurgatoryItem<Spaceship> item;

    ship.swap(item.obj);
    m_shipPurgatory.push_back(std::move(item));
}

Faction* Constellation::getFaction(int id) {
    if (id < 0 || id >= m_factions.size()) return nullptr;
    else return &m_factions[id];
}

void Constellation::reinitAfterLoad() {
    for (auto& star : m_stars) {
        star->reinitAfterLoad(this);
    }
    for (auto& hyperlane : m_hyperlanes) {
        hyperlane->reinitAfterLoad(this);
    }

    for (Faction& faction : m_factions) {
        faction.reinitAfterLoad(this);
    }
}

Star* Constellation::getStarByID(uint32_t id) {
    for (auto& star : m_stars) {
        if (star->getID() == id) {
            return star.get();
        }
    }
    return nullptr;
}

Hyperlane* Constellation::getHyperlaneByID(uint32_t id) {
    for (auto& hyperlane : m_hyperlanes) {
        if (hyperlane->getID() == id) {
            return hyperlane.get();
        }
    }
    return nullptr;
}

Spaceship* Constellation::getShipByID(uint32_t id) {
    for (auto& star : m_stars) {
        for (auto& ship : star->getSpaceships()) {
            if (ship->getID() == id) return ship.get();
        }
    }
    return nullptr;
}

void Constellation::onStart() {
    for (Faction& faction : m_factions) {
        faction.onStart();
    }
}

float Constellation::closestStarDistanceCoords(const sf::Vector2f& targetPos, const std::vector<double>& coords) {
    float closest = distBetweenVecs(targetPos, sf::Vector2f(coords[0], coords[1]));
    for (std::size_t i = 0; i < coords.size(); i += 2) {
        float dist = distBetweenVecs(targetPos, sf::Vector2f(coords[i], coords[i + 1]));
        if (dist < closest) {
            closest = dist;
        }
    }
    return closest;
}

void Constellation::generatePirates() {
    std::vector<Star*> neutralStars = getStarsByAllegiance(-1);

    if (neutralStars.size() > 0) {
        int rnd = Random::randInt(0, neutralStars.size() - 1);
        neutralStars[rnd]->createBuilding("PIRATE_BASE", Random::randVec(0.0f, 10000.0f), nullptr);
        DEBUG_PRINT("Started pirates in " << neutralStars[rnd]->getName());
    }
}

void Constellation::updatePirates() {
    if (m_numUpdates % 20000 == 0) {
        auto pirateBases = getAllBuildingsOfType("PIRATE_BASE");

        if (pirateBases.size() == 0) {
            generatePirates();
        }

        for (Building* pirateBase : pirateBases) {
            // Steal designs
            PirateBaseMod* mod = pirateBase->getMod<PirateBaseMod>();
            mod->findTheftAllegiance(pirateBase->getCurrentStar(), this);

            if (mod->getTheftAllegiance() != -1) {
                mod->stealDesignFrom(getFaction(mod->getTheftAllegiance()));
            }

            // Attack a random faction
            Star* attackTarget = nullptr;
            std::string factionName;
            if (m_factions.size() > 0) {
                int rnd = Random::randInt(0, m_factions.size() - 1);
                attackTarget = m_factions[rnd].getCapital();
                factionName = m_factions[rnd].getName();
            }

            if (attackTarget != nullptr) {
                for (Spaceship* ship : pirateBase->getCurrentStar()->getAllShipsOfAllegiance(-1)) {
                    if (!ship->isCivilian()) {
                        ship->addOrder(TravelOrder(attackTarget));
                    }
                }
                DEBUG_PRINT("Pirates in " << pirateBase->getCurrentStar()->getName() << " are attacking " << factionName);
            }
        }
    }
}

std::vector<Building*> Constellation::getAllBuildingsOfType(const std::string& type) {
    std::vector<Building*> buildings;
    for (auto& star : m_stars) {
        for (auto& building : star->getBuildings()) {
            if (building->getType() == type) {
                buildings.push_back(building.get());
            }
        }
    }
    return buildings;
}

void Constellation::generateOneStarConstellation() {
    m_stars.push_back(std::make_unique<Star>(sf::Vector2f(0.0f, 0.0f)));
}

std::vector<Star*> Constellation::getStarsByAllegiance(int allegiance) {
    std::vector<Star*> stars;

    for (auto& star : m_stars) {
        if (star->getAllegiance() == allegiance) {
            stars.push_back(star.get());
        }
    }

    return stars;
}