#include "gamepch.h"

#include "Constellation.h"
#include "Debug.h"
#include "GameState.h"
#include "Faction.h"
#include "Random.h"

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

void Constellation::draw(sf::RenderWindow& window) {
    for (std::unique_ptr<Hyperlane>& h : m_hyperlanes) {
        h->draw(window);
        h->getBeginStar()->draw(window);
        h->getEndStar()->draw(window);
    }
}

void Constellation::draw(sf::RenderWindow& window, sf::Shader& shader) {
    for (std::unique_ptr<Hyperlane>& h : m_hyperlanes) {
        h->draw(window);
    }
    for (std::unique_ptr<Star>& s : m_stars) {
        s->draw(window, shader);
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
    if (ev.type == sf::Event::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mouseCoords = sf::Mouse::getPosition(window);
        sf::Vector2f mouseCoordsWorld = window.mapPixelToCoords(mouseCoords);

        for (auto& star : m_stars) {
            if (star->isInShapeRadius(mouseCoordsWorld.x, mouseCoordsWorld.y)) {
                star->clicked(ev, state);
            }
        }
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
    for (std::unique_ptr<Star>& s : m_stars) {
        s->setupJumpPoints();
    }
}

Spaceship* Constellation::createShipAtStar(Spaceship ship) {
    m_spaceships.push_back(std::make_unique<Spaceship>(ship));
    m_spaceships.back().get()->getCurrentStar()->addSpaceship(m_spaceships.back().get());
    return m_spaceships.back().get();
}

void Constellation::generateFactions(int numFactions) {
    for (int i = 0; i < numFactions; i++) {
        Faction newFaction(this);
        newFaction.spawnAtRandomStar();
        m_factions.push_back(std::move(newFaction));
    }
}

void Constellation::update() {
    for (Faction& f : m_factions) {
        f.update();
    }
    for (std::unique_ptr<Star>& s : m_stars) {
        s->update();
    }
    cleanUpDeadShips();
}

void Constellation::cleanUpDeadShips() {
    while (m_toBeDeletedShips.size() > 0) {
        m_toBeDeletedShips.front()->getCurrentStar()->removeSpaceship(m_toBeDeletedShips.front().get());
        m_toBeDeletedShips.pop();
    }
    for (int i = 0; i < m_spaceships.size(); i++) {
        if (m_spaceships[i]->isDead()) {
            m_toBeDeletedShips.push(std::move(m_spaceships[i]));
            m_spaceships.erase(m_spaceships.begin() + i);
            i--;
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
                Spaceship ship(Spaceship::SPACESHIP_TYPE::FRIGATE_1, pos, star.get(), -1, sf::Color(175, 175, 175));
                createShipAtStar(ship);
            }
        }
    }
}