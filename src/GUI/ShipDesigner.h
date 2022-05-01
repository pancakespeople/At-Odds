#pragma once
#include "../GUI.h"

class ShipDesignerGUI {
public:
	ShipDesignerGUI() {}

	void open(tgui::Gui& gui, Faction* playerFaction);
	void displayShipInfo(Faction* playerFaction);
	bool canChassisFitWeapons(Faction* playerFaction, std::string& errMsg);
	void displayShipDesigns(Faction* playerFaction);
	void draw(sf::RenderWindow& window);

private:
	void displayChassisInfo(const std::string& chassisType);
	void displayWeaponInfo(const std::string& weaponType);

	GUI::Icon m_icon;
	tgui::ChildWindow::Ptr m_window;
	tgui::Canvas::Ptr m_shipCanvas;
	std::string m_shipTexturePath;
	float m_shipRotation = 0.0f;
};