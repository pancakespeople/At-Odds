#pragma once
#include "../GUI.h"

class ColonyListGUI {
public:
	ColonyListGUI() {}

	void open(tgui::Gui& gui, GameState& state, Constellation& constellation);

	void refreshListViewContents(GameState & state, Constellation & constellation, tgui::ListView::Ptr listView);

private:
	GUI::Icon m_icon;
	tgui::ChildWindow::Ptr m_window = nullptr;
	bool m_systemOnly = false;
};
