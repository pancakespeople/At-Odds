#pragma once
#include "../GUI.h"

class Faction;

class TechGUI {
public:
	TechGUI() {}
	void open(tgui::Gui& gui, Faction* playerFaction);
	void update(Faction* playerFaction);

private:
	GameWidget::Icon m_icon;
	tgui::ChildWindow::Ptr m_window = nullptr;
	tgui::ProgressBar::Ptr m_progressBar = nullptr;
	tgui::ListBox::Ptr m_techQueue = nullptr;
	std::string m_progressBarTech = "";
	tgui::Label::Ptr m_description = nullptr;
};