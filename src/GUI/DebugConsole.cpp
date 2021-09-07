#include "gamepch.h"
#include "DebugConsole.h"
#include "../Faction.h"
#include "../Constellation.h"
#include "../TOMLCache.h"

void spawnShip(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 2) && goodies.console->validateState(command, goodies.state, GameState::State::LOCAL_VIEW)) {
		std::string type = command.args[0];
		int allegiance = std::atoi(command.args[1].c_str());
		sf::Vector2f pos = goodies.window.mapPixelToCoords(sf::Mouse::getPosition(goodies.window));
		Star* star = goodies.state.getLocalViewStar();
		Faction* faction = goodies.constellation.getFaction(allegiance);
		sf::Color color = faction->getColor();

		std::unique_ptr<Spaceship> ship = std::make_unique<Spaceship>(type, pos, star, allegiance, color);
		ship->addWeapon(Weapon("LASER_GUN"));
		faction->addSpaceship(star->createSpaceship(ship));

		goodies.console->addLine("Created spaceship at mouse cursor");
	}
}

void planetDebug(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 0) && goodies.console->validateState(command, goodies.state, GameState::State::LOCAL_VIEW)) {
		std::vector<Planet>& planets = goodies.state.getLocalViewStar()->getPlanets();
		for (int i = 0; i < planets.size(); i++) {
			goodies.console->addLine("Planet " + std::to_string(i));
			goodies.console->addLine("Type: " + planets[i].getTypeString());
			goodies.console->addLine("Temperature: " + std::to_string(planets[i].getTemperature()));
			goodies.console->addLine("Atmospheric Pressure: " + std::to_string(planets[i].getAtmosphericPressure()));
			goodies.console->addLine("Water: " + std::to_string(planets[i].getWater()));
		}
	}
}

void goPlanet(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 1) && goodies.console->validateState(command, goodies.state, GameState::State::LOCAL_VIEW)) {
		int index = std::atoi(command.args[0].c_str());
		std::vector<Planet>& planets = goodies.state.getLocalViewStar()->getPlanets();

		if (index < 0 || index >= planets.size()) {
			goodies.console->addLine("Invalid index");
		}
		else {
			goodies.state.getCamera().setPos(planets[index].getPos());
			goodies.console->addLine("Set camera pos to planet " + command.args[0]);
		}
	}
}

void mostHabitable(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 0) && goodies.console->validateNotState(command, goodies.state, GameState::State::MAIN_MENU)) {
		if (goodies.state.getState() == GameState::State::LOCAL_VIEW) {
			goodies.state.changeToWorldView();
		}
		Star* highestHabitabilityStar = nullptr;
		Planet* highestHabitabilityPlanet = nullptr;
		float highestHabitability = 0.0f;

		for (auto& star : goodies.constellation.getStars()) {
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
			goodies.console->addLine("Didn't find a planet somehow");
		}
		else {
			goodies.state.changeToLocalView(highestHabitabilityStar);
			goodies.state.getCamera().setPos(highestHabitabilityPlanet->getPos());
			goodies.state.getCamera().setAbsoluteZoom(1.0f);
			goodies.console->addLine("Zoomed to the most habitable planet in the constellation");
		}

	}
}

void listFactions(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 0) && goodies.console->validateNotState(command, goodies.state, GameState::State::MAIN_MENU)) {
		int i = 0;
		for (Faction& faction : goodies.constellation.getFactions()) {
			goodies.console->addLine(std::to_string(i) + ": " + faction.getName());
			i++;
		}
	}
}

void possess(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 1) && goodies.console->validateNotState(command, goodies.state, GameState::State::MAIN_MENU)) {
		Faction* faction = goodies.constellation.getFaction(std::atoi(command.args[0].c_str()));
		if (faction != nullptr) {
			goodies.state.getPlayer().setFaction(faction->getID(), faction->getColor());
			goodies.state.getPlayer().enableFogOfWar();

			goodies.console->close(goodies.gui);

			goodies.gui.removeAllWidgets();
			goodies.playerGUI.open(goodies.gui, goodies.state, goodies.constellation, false);
		}
		else {
			goodies.console->addLine("Invalid faction");
		}
	}
}

void spectate(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 0) && goodies.console->validateNotState(command, goodies.state, GameState::State::MAIN_MENU)) {
		goodies.state.getPlayer().setFaction(-1, sf::Color(175, 175, 175));
		goodies.state.getPlayer().disableFogOfWar();

		goodies.console->close(goodies.gui);

		goodies.gui.removeAllWidgets();
		goodies.playerGUI.open(goodies.gui, goodies.state, goodies.constellation, true);
	}
}

void giveWeapon(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 1) && goodies.console->validateNotState(command, goodies.state, GameState::State::MAIN_MENU)) {
		Faction* playerFaction = goodies.constellation.getFaction(goodies.state.getPlayer().getFaction());
		if (playerFaction != nullptr) {
			playerFaction->addWeapon(command.args[0]);
		}
	}
}

void giveResource(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 2) && goodies.console->validateNotState(command, goodies.state, GameState::State::MAIN_MENU)) {
		Faction* playerFaction = goodies.constellation.getFaction(goodies.state.getPlayer().getFaction());
		if (playerFaction != nullptr) {
			playerFaction->addResource(command.args[0], std::stof(command.args[1]));
		}
	}
}

void giveEverything(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 0) && goodies.console->validateNotState(command, goodies.state, GameState::State::MAIN_MENU)) {
		const toml::table& resources = TOMLCache::getTable("data/objects/resources.toml");
		const toml::table& weaponDesigns = TOMLCache::getTable("data/objects/weapondesigns.toml");

		Faction* playerFaction = goodies.constellation.getFaction(goodies.state.getPlayer().getFaction());

		for (auto& elem : resources) {
			playerFaction->addResource(elem.first, 100000);
		}

		for (auto& elem : weaponDesigns) {
			playerFaction->addWeapon(elem.first);
		}
	}
}

void ownPlanet(const DebugConsole::Command& command, const DebugConsole::Goodies& goodies) {
	if (goodies.console->validateArgs(command, 2) && goodies.console->validateState(command, goodies.state, GameState::State::LOCAL_VIEW)) {
		Planet& planet = goodies.state.getLocalViewStar()->getPlanets()[std::stoi(command.args[0])];
		int allegiance = std::stoi(command.args[1]);
		Faction* faction = goodies.constellation.getFaction(allegiance);

		if (planet.getColony().getPopulation() == 0) {
			planet.onColonization();
			planet.getColony().addPopulation(1000);
		}

		planet.getColony().setFactionColor(faction->getColor());
		planet.getColony().setAllegiance(allegiance);
	}
	else {
		goodies.console->addLine("usage: ownplanet {planet index} {allegiance}");
	}
}

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

	addCommand("spawnship", spawnShip);
	addCommand("planetdebug", planetDebug);
	addCommand("goplanet", goPlanet);
	addCommand("mosthabitable", mostHabitable);
	addCommand("listfactions", listFactions);
	addCommand("possess", possess);
	addCommand("spectate", spectate);
	addCommand("giveweapon", giveWeapon);
	addCommand("giveresource", giveResource);
	addCommand("giveeverything", giveEverything);
	addCommand("ownplanet", ownPlanet);
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

		if (m_commands.count(command.command) > 0) {
			addLine("Running command " + command.command);
			m_commands[command.command](command, DebugConsole::Goodies{ this, constellation, state, window, gui, playerGUI });
		}
		else {
			addLine("Invalid command " + command.command);
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