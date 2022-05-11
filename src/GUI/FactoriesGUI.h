#pragma once
#include "../GUI.h"
#include "../Mod.h"
#include "ShipFactoryGUI.h"

class FactoriesGUI {
public:
	void open(tgui::Gui& gui, Faction& faction);
	void draw(sf::RenderWindow& window);
private:
	GUI::Icon m_icon;
	tgui::ChildWindow::Ptr m_window;
	tgui::Panel::Ptr m_shipFactoryPanel;
	ShipFactoryGUI m_shipFactoryGUI;
};