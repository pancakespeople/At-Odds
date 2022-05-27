#pragma once
#include "../GUI.h"

class Faction;

class TechGUI {
public:
	TechGUI() {}
	void open(tgui::Gui& gui, Faction* playerFaction);
	void updateTechs(Faction * playerFaction);
	void update(Faction* playerFaction);
	void draw(sf::RenderWindow& window);

private:
	GUI::Icon m_icon;
	GUI::ChildWindow::Ptr m_window = nullptr;
	tgui::ProgressBar::Ptr m_progressBar = nullptr;
	tgui::ListBox::Ptr m_techQueue = nullptr;
	std::string m_progressBarTech = "";
	tgui::Label::Ptr m_description = nullptr;
	tgui::ListBox::Ptr m_researchableTechs = nullptr;
	tgui::Tabs::Ptr m_tabs = nullptr;
};