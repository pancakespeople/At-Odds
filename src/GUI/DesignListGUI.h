#pragma once
#include "../GUI.h"
#include "../Mod.h"

class DesignListGUI {
public:
	void open(tgui::Gui& gui, Faction& faction, tgui::Group* group);
	void updateDesigns(Faction* faction);
	void updateDesignsListBox(int selectedIndex);
	
	std::deque<FactoryMod::ShipBuildData> shipBuildData;

private:
	//tgui::ProgressBar::Ptr m_buildProgressBar;
	tgui::Group::Ptr m_shipWidgets;
	tgui::Label::Ptr m_armamentsLabel;
	tgui::Label::Ptr m_buildSpeedLabel;
	tgui::ListBox::Ptr m_designsListBox;
};