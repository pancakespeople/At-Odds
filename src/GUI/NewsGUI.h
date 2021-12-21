#pragma once

class Faction;

class NewsGUI {
public:
	NewsGUI() = default;
	void open(tgui::Gui& gui);
	void addEvent(const std::string& text, sf::Color color);
	void update(Faction* playerFaction);

private:
	tgui::ChatBox::Ptr m_box;
};