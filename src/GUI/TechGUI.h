#pragma once
#include "../GUI.h"

class Faction;

class TechGUI {
public:
	TechGUI() {}
	void open(tgui::Gui& gui, Faction* playerFaction);

private:
	GameWidget::Icon m_icon;
};