#include "gamepch.h"
#include "Derelict.h"
#include "TextureCache.h"
#include "Random.h"
#include "Star.h"
#include "TOMLCache.h"
#include "Spaceship.h"

Derelict::Derelict(sf::Vector2f pos) {
	m_sprite.setTexture(TextureCache::getTexture("data/art/derelict.png"));
	m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f, m_sprite.getLocalBounds().height / 2.0f);
	m_sprite.setPosition(pos);
	m_sprite.setRotation(Random::randFloat(0.0f, 360.0f));
	m_sprite.setScale(2.0f, 2.0f);

	m_collider = Collider(pos, sf::Color(175, 175, 175), m_sprite.getLocalBounds().width * m_sprite.getScale().x / 1.5f);
}

void Derelict::draw(sf::RenderWindow& window) {
	window.draw(m_sprite);
	window.draw(m_collider);
}

void Derelict::update(Star* star, std::vector<Faction>& factions) {
	for (auto& ship : star->getSpaceships()) {
		if (ship->getAllegiance() != -1) {
			if (m_collider.isCollidingWith(ship->getCollider()) && !m_dead) {
				
				for (Faction& faction : factions) {
					if (faction.getID() == ship->getAllegiance()) {
						const toml::table& table = TOMLCache::getTable("data/objects/weapondesigns.toml");
						
						if (table.size() > 0) {
							std::vector<std::string> vals;

							for (auto& elem : table) {
								vals.push_back(elem.first);
							}
							
							int randIdx = Random::randInt(0, vals.size() - 1);
							Spaceship::DesignerWeapon weapon(vals[randIdx]);

							bool alreadyHas = false;
							for (auto& w : faction.getWeapons()) {
								if (w.name == weapon.name) {
									alreadyHas = true;
									break;
								}
							}

							if (!alreadyHas) faction.addWeapon(weapon);
							faction.addAnnouncementEvent("You plundered a derelict and found a " + weapon.name + " design");
						}
					}
				}
				
				m_dead = true;
			}
		}
	}
}