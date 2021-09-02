#pragma once

#include "../GUI.h"

class DebugConsole {
public:
	struct Command {
		std::string command;
		std::vector<std::string> args;
	};

	DebugConsole() {}

	void open(tgui::Gui& gui);
	void close(tgui::Gui& gui);
	void onEvent(sf::Event& ev, tgui::Gui& gui, GameState& state);
	void processCommand(std::string rawCommand);
	void runCommands(Constellation& constellation, GameState& state, sf::RenderWindow& window, tgui::Gui& gui, PlayerGUI& playerGUI);

	bool isOpen() { return m_console != nullptr; }
	bool validateArgs(const Command& command, int numArgs);
	bool validateState(const Command& command, const GameState& state, GameState::State requestedState);
	bool validateNotState(const Command& command, const GameState& state, GameState::State notState);

private:
	tgui::Group::Ptr m_console;
	tgui::ChatBox::Ptr m_chatBox;
	tgui::EditBox::Ptr m_editBox;

	std::queue<Command> m_commandQueue;
};