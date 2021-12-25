#include "gamepch.h"
#include "ColonyListGUI.h"
#include "../GameState.h"
#include "../Constellation.h"

void ColonyListGUI::open(tgui::Gui& gui, GameState& state, Constellation& constellation) {
	m_icon.open(gui, tgui::Layout2d("0%", "75%"), tgui::Layout2d("2.5%", "5%"), "data/art/colonyicon.png");

	m_icon.panel->onClick([this, &gui, &state, &constellation]() {
		if (m_window == nullptr) {
			m_window = tgui::ChildWindow::create("Colony List");
			m_window->setSize("50%", "50%");
			m_window->setOrigin(0.5, 0.5);
			m_window->setPosition(gui.getTarget()->getSize().x / 2.0, gui.getTarget()->getSize().y / 2.0);
			m_window->getRenderer()->setOpacity(0.75f);
			m_window->onClose([this]() {
				m_window = nullptr;
				});
			gui.add(m_window);

			auto listView = tgui::ListView::create();
			listView->setSize("100%", "95%");
			listView->setPosition("0%", "5%");
			listView->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
			listView->addColumn("Type    ");
			listView->addColumn("Name");
			listView->addColumn("Population");
			listView->addColumn("Growth Rate");
			listView->addColumn("Temperature");
			listView->addColumn("Water");
			listView->addColumn("Atmosphere");
			listView->addColumn("Habitability");
			m_window->add(listView);

			auto refreshButton = tgui::Button::create("Refresh");
			refreshButton->setSize("10%", "5%");
			m_window->add(refreshButton);

			auto systemOnlyCheckbox = tgui::CheckBox::create("Current System Only");
			systemOnlyCheckbox->setPosition("12.5%", "0%");
			systemOnlyCheckbox->setChecked(m_systemOnly);
			m_window->add(systemOnlyCheckbox);

			listView->onHeaderClick([listView](int index) {
				listView->sort(index, [](const tgui::String& a, const tgui::String& b) {
					float aFloat = a.toFloat();
					float bFloat = b.toFloat();
					return aFloat > bFloat;
					});
				});

			listView->onItemSelect([listView, &state, &constellation](int index) {
				if (index == -1) return;
				std::pair<int, int> starPlanetPair = listView->getItemData<std::pair<int, int>>(index); // IDs
				Star* star = constellation.getStarByID(starPlanetPair.first);
				if (star != nullptr) {
					Planet* planet = star->getPlanetByID(starPlanetPair.second);
					if (planet != nullptr) {
						if (state.getState() == GameState::State::LOCAL_VIEW) {
							state.changeToWorldView();
							state.changeToLocalView(star);
						}
						else {
							state.changeToLocalView(star);
						}
						state.getCamera().setPos(planet->getPos());
					}
				}
			});

			refreshButton->onClick([this, listView, &state, &constellation]() {
				refreshListViewContents(state, constellation, listView);
			});

			systemOnlyCheckbox->onChange([this, listView, &state, &constellation](bool checked) {
				m_systemOnly = checked;
				refreshListViewContents(state, constellation, listView);
			});

			refreshListViewContents(state, constellation, listView);
		}
		else {
			gui.remove(m_window);
			m_window = nullptr;
		}
		});
}

void ColonyListGUI::refreshListViewContents(GameState & state, Constellation & constellation, tgui::ListView::Ptr listView) {
	int playerFaction = state.getPlayer().getFaction();
	listView->removeAllItems();

	for (auto& star : constellation.getStars()) {
		int maxNameLength = (star->getName().length() + 5) * 10;
		listView->setColumnWidth(1, maxNameLength);
		for (Planet& planet : star->getPlanets()) {
			if (m_systemOnly && state.getLocalViewStar() != star.get()) {
				continue;
			}
			if (planet.getColony().getAllegiance() == playerFaction) {
				std::vector<tgui::String> info;
				info.push_back(planet.getTypeString());
				info.push_back(planet.getName(star.get()));
				info.push_back(std::to_string(planet.getColony().getPopulation()));
				info.push_back(std::to_string(planet.getColony().getGrowthRate(planet.getHabitability()) * 100.0f) + "%");
				info.push_back(std::to_string(planet.getTemperature()));
				info.push_back(std::to_string(planet.getWater()));
				info.push_back(std::to_string(planet.getAtmosphericPressure()));
				info.push_back(std::to_string(planet.getHabitability()));
				listView->addItem(info);
				listView->setItemData(listView->getItemCount() - 1, std::pair<int, int>(star->getID(), planet.getID()));
			}
		}
	}
}
