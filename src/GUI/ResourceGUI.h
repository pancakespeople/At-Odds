#pragma once
#include "../GUI.h"

class ResourceGUI {
public:
	void open(tgui::Gui& gui);
	void update(Constellation& constellation, Player& player);

private:
	tgui::Group::Ptr m_resourceGroup;
	tgui::Panel::Ptr m_panel;
	
	tgui::Label::Ptr m_commonLabel;
	tgui::Label::Ptr m_uncommonLabel;
	tgui::Label::Ptr m_rareLabel;

	int m_timeUntilChangesUpdate = 0;
	float m_commonChange = 0.0f;
	float m_uncommonChange = 0.0f;
	float m_rareChange = 0.0f;
};