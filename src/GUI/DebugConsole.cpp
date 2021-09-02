#include "gamepch.h"
#include "DebugConsole.h"
#include "../Faction.h"
#include "../Constellation.h"
#include "../TOMLCache.h"

void DebugConsole::open(tgui::Gui& gui) {
	m_console = tgui::Group::create();
	m_console->getRenderer()->setOpacity(0.75f);

	m_chatBox = tgui::ChatBox::create();
	m_chatBox->setSize("33%", "25%");
	m_chatBox->setTextColor(tgui::Color::White);
	m_console->add(m_chatBox);

	m_editBox = tgui::EditBox::create();
	m_editBox->setPosition("0%", "25%");
	m_editBox->setSize("33%", "5%");
	m_editBox->onReturnKeyPress([this]() {
		m_chatBox->addLine(">>> " + m_editBox->getText());
		processCommand(m_editBox->getText().toStdString());
		m_editBox->setText("");
		});
	m_console->add(m_editBox);

	gui.add(m_console);
}

void DebugConsole::close(tgui::Gui& gui) {
	if (m_console != nullptr) {
		gui.remove(m_console);
		m_console = nullptr;
	}
}

void DebugConsole::onEvent(sf::Event& ev, tgui::Gui& gui, GameState& state) {
	if (ev.type == sf::Event::EventType::KeyReleased && ev.key.code == sf::Keyboard::Tilde && state.getState() != GameState::State::MAIN_MENU) {
		if (isOpen()) {
			close(gui);
		}
		else {
			open(gui);
		}
	}
}

void DebugConsole::processCommand(std::string rawCommand) {
	std::stringstream strs(rawCommand);
	std::string word;
	std::vector<std::string> split;

	while (strs >> word) {
		split.push_back(word);
	}

	if (split.size() == 0) return;

	std::string command = split[0];
	std::vector<std::string> args;

	if (split.size() > 1) {
		args.insert(args.begin(), split.begin() + 1, split.end());
	}

	Command c;
	c.command = command;
	c.args = args;

	m_commandQueue.push(c);
}

void DebugConsole::runCommands(Constellation& constellation, GameState& state, sf::RenderWindow& window, tgui::Gui& gui, PlayerGUI& playerGUI) {
	while (m_commandQueue.size() > 0) {
		Command command = m_commandQueue.front();
		m_commandQueue.pop();

		m_chatBox->addLine("Running command " + command.command);

		if (command.command == "spawnship") {
			if (validateArgs(command, 2) && validateState(command, state, GameState::State::LOCAL_VIEW)) {
				std::string type = command.args[0];
				int allegiance = std::atoi(command.args[1].c_str());
				sf::Vector2f pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
				Star* star = state.getLocalViewStar();
				Faction* faction = constellation.getFaction(allegiance);
				sf::Color color = faction->getColor();

				std::unique_ptr<Spaceship> ship = std::make_unique<Spaceship>(type, pos, star, allegiance, color);
				ship->addWeapon(Weapon("LASER_GUN"));
				faction->addSpaceship(star->createSpaceship(ship));

				m_chatBox->addLine("Created spaceship at mouse cursor");
			}
		}
		else if (command.command == "planetdebug") {
			if (validateArgs(command, 0) && validateState(command, state, GameState::State::LOCAL_VIEW)) {
				std::vector<Planet>& planets = state.getLocalViewStar()->getPlanets();
				for (int i = 0; i < planets.size(); i++) {
					m_chatBox->addLine("Planet " + std::to_string(i));
					m_chatBox->addLine("Type: " + planets[i].getTypeString());
					m_chatBox->addLine("Temperature: " + std::to_string(planets[i].getTemperature()));
					m_chatBox->addLine("Atmospheric Pressure: " + std::to_string(planets[i].getAtmosphericPressure()));
					m_chatBox->addLine("Water: " + std::to_string(planets[i].getWater()));
				}
			}
		}
		else if (command.command == "goplanet") {
			if (validateArgs(command, 1) && validateState(command, state, GameState::State::LOCAL_VIEW)) {
				int index = std::atoi(command.args[0].c_str());
				std::vector<Planet>& planets = state.getLocalViewStar()->getPlanets();

				if (index < 0 || index >= planets.size()) {
					m_chatBox->addLine("Invalid index");
				}
				else {
					state.getCamera().setPos(planets[index].getPos());
					m_chatBox->addLine("Set camera pos to planet " + command.args[0]);
				}
			}
		}
		else if (command.command == "mosthabitable") {
			if (validateArgs(command, 0) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				if (state.getState() == GameState::State::LOCAL_VIEW) {
					state.changeToWorldView();
				}
				Star* highestHabitabilityStar = nullptr;
				Planet* highestHabitabilityPlanet = nullptr;
				float highestHabitability = 0.0f;

				for (auto& star : constellation.getStars()) {
					for (auto& planet : star->getPlanets()) {
						float habitability = planet.getHabitability();
						if (habitability > highestHabitability) {
							highestHabitability = habitability;
							highestHabitabilityStar = star.get();
							highestHabitabilityPlanet = &planet;
						}
					}
				}

				if (highestHabitabilityPlanet == nullptr) {
					m_chatBox->addLine("Didn't find a planet somehow");
				}
				else {
					state.changeToLocalView(highestHabitabilityStar);
					state.getCamera().setPos(highestHabitabilityPlanet->getPos());
					state.getCamera().setAbsoluteZoom(1.0f);
					m_chatBox->addLine("Zoomed to the most habitable planet in the constellation");
				}

			}
		}
		else if (command.command == "listfactions") {
			if (validateArgs(command, 0) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				int i = 0;
				for (Faction& faction : constellation.getFactions()) {
					m_chatBox->addLine(std::to_string(i) + ": " + faction.getName());
					i++;
				}
			}
		}
		else if (command.command == "possess") {
			if (validateArgs(command, 1) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				Faction* faction = constellation.getFaction(std::atoi(command.args[0].c_str()));
				if (faction != nullptr) {
					state.getPlayer().setFaction(faction->getID(), faction->getColor());
					state.getPlayer().enableFogOfWar();

					close(gui);

					gui.removeAllWidgets();
					playerGUI.open(gui, state, constellation, false);
				}
				else {
					m_chatBox->addLine("Invalid faction");
				}
			}
		}
		else if (command.command == "spectate") {
			if (validateArgs(command, 0) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				state.getPlayer().setFaction(-1, sf::Color(175, 175, 175));
				state.getPlayer().disableFogOfWar();

				close(gui);

				gui.removeAllWidgets();
				playerGUI.open(gui, state, constellation, true);
			}
		}
		else if (command.command == "giveweapon") {
			if (validateArgs(command, 1) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				Faction* playerFaction = constellation.getFaction(state.getPlayer().getFaction());
				if (playerFaction != nullptr) {
					playerFaction->addWeapon(command.args[0]);
				}
			}
		}
		else if (command.command == "giveresource") {
			if (validateArgs(command, 2) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				Faction* playerFaction = constellation.getFaction(state.getPlayer().getFaction());
				if (playerFaction != nullptr) {
					playerFaction->addResource(command.args[0], std::stof(command.args[1]));
				}
			}
		}
		else if (command.command == "giveeverything") {
			if (validateArgs(command, 0) && validateNotState(command, state, GameState::State::MAIN_MENU)) {
				const toml::table& resources = TOMLCache::getTable("data/objects/resources.toml");
				const toml::table& weaponDesigns = TOMLCache::getTable("data/objects/weapondesigns.toml");

				Faction* playerFaction = constellation.getFaction(state.getPlayer().getFaction());

				for (auto& elem : resources) {
					playerFaction->addResource(elem.first, 100000);
				}

				for (auto& elem : weaponDesigns) {
					playerFaction->addWeapon(elem.first);
				}
			}
		}
		else {
			m_chatBox->addLine("Invalid command " + command.command);
		}
	}
}

bool DebugConsole::validateArgs(const Command& command, int numArgs) {
	if (command.args.size() == numArgs) {
		return true;
	}
	else {
		m_chatBox->addLine("Invalid arguments for command " + command.command);
		return false;
	}
}

bool DebugConsole::validateState(const Command& command, const GameState& state, GameState::State requestedState) {
	if (state.getState() == requestedState) {
		return true;
	}
	else {
		m_chatBox->addLine("Invalid game state for command " + command.command);
		return false;
	}
}

bool DebugConsole::validateNotState(const Command& command, const GameState& state, GameState::State notState) {
	if (state.getState() != notState) {
		return true;
	}
	else {
		m_chatBox->addLine("Invalid game state for command " + command.command);
		return false;
	}
}