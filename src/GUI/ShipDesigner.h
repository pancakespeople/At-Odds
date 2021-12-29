#pragma once
#include "../GUI.h"

class ShipDesignerGUI {
public:
	ShipDesignerGUI() {}

	void open(tgui::Gui& gui, Faction* playerFaction);
	void displayShipInfo(Faction* playerFaction);
	bool canChassisFitWeapons(Faction* playerFaction);
	void displayShipDesigns(Faction* playerFaction);

private:
	void displayChassisInfo(const std::string& chassisType);

	GameWidget::Icon m_icon;
	tgui::ChildWindow::Ptr m_window;
};