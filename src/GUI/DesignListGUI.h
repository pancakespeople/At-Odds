#pragma once
#include "../GUI.h"
#include "../Mod.h"

class DesignListGUI {
public:
	void open(tgui::Gui& gui, Faction& faction, tgui::Group* group);
	void updateDesigns(Faction* faction);
	void updateDesignsListBox(int selectedIndex);

private:
	std::deque<FactoryMod::ShipBuildData> m_shipBuildData;
	//tgui::ProgressBar::Ptr m_buildProgressBar;
	tgui::Group::Ptr m_shipWidgets;
	tgui::Label::Ptr m_armamentsLabel;
	tgui::Label::Ptr m_buildSpeedLabel;
	tgui::ListBox::Ptr m_designsListBox;
};