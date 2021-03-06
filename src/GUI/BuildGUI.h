#pragma once
#include "../GUI.h"

class Faction;

class BuildGUI {
public:
	struct BuildingSelector {
		BuildingPrototype prototype;
		tgui::Picture::Ptr icon;
		tgui::Panel::Ptr panel;
	};

	BuildGUI() {}

	void open(tgui::Gui& gui, Faction* playerFaction);
	void draw(sf::RenderWindow& window, Renderer& renderer, Star* currentStar, Faction* playerFaction);
	void onEvent(const sf::Event& ev, const sf::RenderWindow& window, Renderer& renderer, Star* currentLocalStar, Faction* playerFaction, UnitGUI& unitGUI, tgui::Panel::Ptr mainPanel);

private:
	void onBuildIconMouseEnter();
	void onBuildIconMouseExit();
	void onBuildIconClick(tgui::Gui& gui, Faction* playerFaction);
	void addBuildingSelector(const std::string& type);
	void onBuildingSelectorMouseEnter(int selectorIdx);
	void onBuildingSelectorMouseExit(int selectorIdx);
	void onBuildingSelectorClick(int selectorIdx);
	void updateBuildingSelectors(Faction* playerFaction);
	void updateBuildInfo(const BuildingSelector& selector);

	GUI::Icon m_buildIcon;
	tgui::Panel::Ptr m_buildPanel;
	tgui::Panel::Ptr m_buildInfoPanel;
	tgui::Tabs::Ptr m_tabs;
	std::vector<BuildingSelector> m_buildingSelectors;

	int m_selectedBuildingIdx = -1;
	bool m_canReceiveEvents = true;
};