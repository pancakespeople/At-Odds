#pragma once
#include "../GUI.h"
#include "../Mod.h"
#include "DesignListGUI.h"

class FactoriesGUI {
public:
	void open(tgui::Gui& gui, Faction& faction);
	void draw(sf::RenderWindow& window);
	void update(Constellation& constellation, Faction* playerFaction);
private:
	GUI::Icon m_icon;
	tgui::ChildWindow::Ptr m_window;
	tgui::Panel::Ptr m_shipFactoryPanel;
	DesignListGUI m_designListGUI;
	tgui::Label::Ptr m_infoLabel;
	tgui::Label::Ptr m_factoriesToUpdateLabel;
	tgui::Slider::Ptr m_slider;
};