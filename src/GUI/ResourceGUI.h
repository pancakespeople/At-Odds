#pragma once
#include "../GUI.h"

class ResourceGUI {
public:
	void open(tgui::Gui& gui);
	void update(Constellation& constellation, Player& player, Star* currentStar);

private:
	tgui::Panel::Ptr m_panel;
	
	tgui::Label::Ptr m_commonLabel;
	tgui::Label::Ptr m_uncommonLabel;
	tgui::Label::Ptr m_rareLabel;
	tgui::Label::Ptr m_scienceLabel;
	tgui::Label::Ptr m_populationLabel;

	int m_timeUntilUpdate = 0;
	float m_commonChange = 0.0f;
	float m_uncommonChange = 0.0f;
	float m_rareChange = 0.0f;

	float m_common = 0.0f;
	float m_uncommon = 0.0f;
	float m_rare = 0.0f;
};