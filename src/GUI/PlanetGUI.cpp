#include "gamepch.h"
#include "PlanetGUI.h"
#include "../Faction.h"
#include "../Star.h"
#include "../Math.h"
#include "../TOMLCache.h"
#include "../Util.h"
#include "../Renderer.h"
#include "../Constellation.h"

void PlanetGUI::open(tgui::Gui& gui, GameState& state, Faction* playerFaction, const Constellation& constellation) {
	if (m_planetPanel != nullptr) {
		gui.remove(m_planetPanel);
		gui.remove(m_buttonPanel);
		m_planetPanel = nullptr;
		m_buttonPanel = nullptr;
	}

	m_planetIcon.open(gui, { "0%", "85%" }, { "2.5%", "5%" }, "data/art/planetsicon.png");

	m_planetIcon.panel->onMouseEnter([this]() {
		m_planetIcon.panel->getRenderer()->setBackgroundColor(tgui::Color::White);
		m_planetIcon.panel->setRenderer(tgui::Theme().getRenderer("Panel"));
		});

	m_planetIcon.panel->onMouseLeave([this]() {
		m_planetIcon.panel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
		m_planetIcon.panel->getRenderer()->setOpacity(0.75f);
		m_planetIcon.panel->setRenderer(tgui::Theme::getDefault()->getRenderer("Panel"));
	});

	m_planetIcon.panel->onClick([this, &gui, &state, &constellation, playerFaction]() {
		if (m_planetPanel == nullptr) {
			if (state.getLocalViewStar() != nullptr) {
				m_planetPanel = tgui::ChildWindow::create("Planets");
				m_planetPanel->setInheritedOpacity(0.75);
				m_planetPanel->setPosition("2.5%", "61%");
				m_planetPanel->setSize("20%", "29%");
				m_planetPanel->onClose([this, &gui]() {
					closePanel(gui);
				});
				gui.add(m_planetPanel, "planetPanel");

				m_planetInfoPanel = tgui::Panel::create();
				m_planetInfoPanel->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
				m_planetInfoPanel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
				m_planetInfoPanel->getRenderer()->setBorders(1.0f);
				m_planetInfoPanel->setPosition("0%", "20%");
				m_planetInfoPanel->setSize("100%", "80%");
				m_planetPanel->add(m_planetInfoPanel);

				m_buttonPanel = tgui::Panel::create();
				m_buttonPanel->setPosition("planetPanel.right", "planetPanel.y");
				m_buttonPanel->setSize("10%", "planetPanel.height");
				gui.add(m_buttonPanel, "buttonPanel");

				std::vector<Planet>& planets = state.getLocalViewStar()->getPlanets();

				auto planetList = tgui::ComboBox::create();
				planetList->setPosition("25%", "5%");
				planetList->setSize("50%", "10%");
				planetList->onItemSelect([this, planetList, &state, &gui, &constellation, playerFaction]() {
					setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getSelectedItemIndex(), constellation);
					});
				m_planetPanel->add(planetList, "planetList");

				// Add planets to dropdown list if star is discovered
				int playerAllegiance = -1;
				if (playerFaction != nullptr) {
					playerAllegiance = playerFaction->getID();
				}

				if (state.getLocalViewStar()->isDiscovered(playerAllegiance)) {
					for (int i = 0; i < planets.size(); i++) {
						planetList->addItem(planets[i].getName(state.getLocalViewStar()));
					}
				}

				// Set first planet as selected
				if (planetList->getItemCount() > 0) {
					setSelectedPlanet(planetList, state, playerFaction, gui, 0, constellation);
				}

				auto backButton = GUI::Button::create("<-");
				backButton->setPosition("5%", "5%");
				backButton->onClick([this, planetList, &state, &gui, &constellation, playerFaction]() {
					if (planetList->getItemCount() > 1) {
						if (planetList->getSelectedItemIndex() == 0) {
							setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getItemCount() - 1, constellation);
						}
						else {
							setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getSelectedItemIndex() - 1, constellation);
						}
					}
					});
				m_planetPanel->add(backButton);

				auto forwardButton = GUI::Button::create("->");
				forwardButton->setPosition("84%", "5%");
				forwardButton->onClick([this, planetList, &state, &gui, &constellation, playerFaction]() {
					if (planetList->getItemCount() > 1) {
						if (planetList->getSelectedItemIndex() == planetList->getItemCount() - 1) {
							setSelectedPlanet(planetList, state, playerFaction, gui, 0, constellation);
						}
						else {
							setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getSelectedItemIndex() + 1, constellation);
						}
					}
					});
				m_planetPanel->add(forwardButton);
			}
		}
		else {
			closePanel(gui);
		}
		});

	state.addOnChangeStateCallback([this, &gui]() {
		closePanel(gui);
		});
}

void PlanetGUI::setSelectedPlanet(tgui::ComboBox::Ptr planetList, GameState& state, Faction* playerFaction, tgui::Gui& gui, int index, const Constellation& constellation) {
	planetList->setSelectedItemByIndex(index);
	m_planetInfoPanel->removeAllWidgets();

	gui.remove(m_sideWindow);
	m_sideWindow = nullptr;

	Planet& planet = state.getLocalViewStar()->getPlanets()[index];

	// Labels

	auto planetTypeLabel = tgui::Label::create();
	planetTypeLabel->setText("Type: " + planet.getTypeString());
	planetTypeLabel->setPosition("0%", "5%");
	m_planetInfoPanel->add(planetTypeLabel);

	auto planetTemperatureLabel = tgui::Label::create();
	planetTemperatureLabel->setText("Temperature: " + Util::cutOffDecimal(planet.getTemperature(), 2) + " (" + planet.getTemperatureText() + ")");
	planetTemperatureLabel->setPosition("0%", "15%");
	m_planetInfoPanel->add(planetTemperatureLabel);

	auto planetAtmosLabel = tgui::Label::create();
	planetAtmosLabel->setText("Atmosphere: " + Util::cutOffDecimal(planet.getAtmosphericPressure(), 2) + " (" + planet.getAtmosphereText() + ")");
	planetAtmosLabel->setPosition("0%", "25%");
	m_planetInfoPanel->add(planetAtmosLabel);

	auto planetWaterLabel = tgui::Label::create();
	planetWaterLabel->setText("Water: " + Util::cutOffDecimal(planet.getWater(), 2) + +" (" + planet.getWaterText() + ")");
	planetWaterLabel->setPosition("0%", "35%");
	m_planetInfoPanel->add(planetWaterLabel);

	auto planetHabitabilityLabel = tgui::Label::create();
	planetHabitabilityLabel->setText("Habitability: " + Util::cutOffDecimal(planet.getHabitability(), 2) + " (" + planet.getHabitabilityText() + ")");
	planetHabitabilityLabel->setPosition("0%", "45%");
	m_planetInfoPanel->add(planetHabitabilityLabel);

	// Buttons

	createColonyAndResourcesButtons(gui, state, planet, playerFaction, constellation);

	if (state.getPlayer().getFaction() != -1) {
		createLawsButton(gui, state, planet);
	}

	createBuildingsButton(gui, planet, playerFaction);
	createEventsButton(gui, planet);
	createTradeButton(gui, planet);

	// Focus camera
	state.getCamera().setPos(planet.getPos());
}

void PlanetGUI::update(GameState& state) {
	if (m_planetPanel != nullptr) {
		auto planetList = m_planetPanel->get<tgui::ComboBox>("planetList");
		if (planetList->getItemCount() > 0) {
			Planet& planet = state.getLocalViewStar()->getPlanets()[planetList->getSelectedItemIndex()];

			// Lock camera on planet
			state.getCamera().setPos(planet.getPos());

			if (m_updateFunction != nullptr) m_updateFunction(planet);
		}

		if (m_sideWindow != nullptr) {
			if (m_sideWindow->getTitle() == "Buildings") {
				auto buildingsBox = m_sideWindow->get<tgui::ListBox>("buildingsBox");

				if (buildingsBox->getSelectedItemIndex() != -1) {
					Planet& planet = state.getLocalViewStar()->getPlanets()[planetList->getSelectedItemIndex()];
					const std::string& type = buildingsBox->getItemData<const std::string&>(buildingsBox->getSelectedItemIndex());

					for (ColonyBuilding& building : planet.getColony().getBuildings()) {
						if (building.getType() == type) {
							// Update progress bar
							m_sideWindow->get<tgui::ProgressBar>("buildProgressBar")->setValue(building.getPercentBuilt());
						}
					}

				}
			}
		}
	}
}

void PlanetGUI::switchSideWindow(const std::string& name, tgui::Gui& gui) {
	if (m_sideWindow == nullptr) {
		m_sideWindow = tgui::ChildWindow::create(name);
		m_sideWindow->setOrigin(0.0f, 1.0f);
		m_sideWindow->setPosition("buttonPanel.right", "buttonPanel.bottom");
		m_sideWindow->setSize("20%", "29%");
		m_sideWindow->getRenderer()->setOpacity(0.75f);
		m_sideWindow->setPositionLocked(true);
		m_sideWindow->onClose([this]() {
			m_sideWindow = nullptr;
			m_updateFunction = nullptr;
			});
		gui.add(m_sideWindow);
	}
	else {
		if (m_sideWindow->getTitle() != name) {
			m_sideWindow->setTitle(name);
			m_sideWindow->setSize("20%", "29%");
			m_sideWindow->removeAllWidgets();
		}
		else {
			m_sideWindow->close();
		}
	}
	m_updateFunction = nullptr;
}

void PlanetGUI::onEvent(const sf::Event& ev, tgui::Gui& gui, GameState& state, Faction* playerFaction, const sf::RenderWindow& window, Renderer& renderer, Star* currentStar, tgui::Panel::Ptr mainPanel, const Constellation& constellation) {
	if (currentStar != nullptr && m_planetIcon.panel != nullptr && mainPanel->isFocused()) {
		static sf::Vector2f lastMousePressPos;

		if (ev.type == sf::Event::EventType::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left) {
			sf::Vector2i screenPos = sf::Mouse::getPosition(window);
			sf::Vector2f worldPos = renderer.mapPixelToCoords(screenPos);

			int i = 0;
			for (Planet& planet : currentStar->getPlanets()) {
				if (Math::distance(worldPos, planet.getPos()) < planet.getRadius() &&
					Math::distance(lastMousePressPos, planet.getPos()) < planet.getRadius()) {
					// Open planet panel
					if (m_planetPanel == nullptr) {
						m_planetIcon.panel->onClick.emit(m_planetIcon.panel.get(), worldPos);
					}
					setSelectedPlanet(m_planetPanel->get<tgui::ComboBox>("planetList"), state, playerFaction, gui, i, constellation);
					break;
				}
				i++;
			}
		}
		else if (ev.type == sf::Event::EventType::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
			lastMousePressPos = renderer.mapPixelToCoords(sf::Mouse::getPosition(window));
		}
	}
}

void PlanetGUI::createColonyAndResourcesButtons(tgui::Gui& gui, GameState& state, Planet& planet, Faction* playerFaction, const Constellation& constellation) {
	// Colony button
	auto colonyInfoButton = GUI::Button::create();
	colonyInfoButton->setPosition("0%", "16.66% * 5");
	colonyInfoButton->setText("Colony");
	colonyInfoButton->setSize("100%", "16.66%");
	auto openColonyInfo = [this, &gui, &state, &constellation, &planet]() {
		switchSideWindow("Colony", gui);

		if (m_sideWindow == nullptr) return;

		Colony& colony = planet.getColony();

		auto populationLabel = tgui::Label::create("Population: " + std::to_string(colony.getPopulation()));
		m_sideWindow->add(populationLabel);

		if (colony.getPopulation() > 0) {
			auto allegianceLabel = tgui::Label::create("Allegiance: ");
			allegianceLabel->setPosition("0%", "10%");
			m_sideWindow->add(allegianceLabel, "allegianceLabel");

			auto allegianceText = tgui::Label::create();
			if (state.getPlayer().getFaction() == colony.getAllegiance()) {
				allegianceText->setText("Friendly");
				allegianceText->getRenderer()->setTextColor(tgui::Color::Green);
			}
			else if (constellation.getAlliances().isAllied(state.getPlayer().getFaction(), colony.getAllegiance())) {
				allegianceText->setText("Allied");
				allegianceText->getRenderer()->setTextColor(tgui::Color::Blue);
			}
			else {
				allegianceText->setText("Hostile");
				allegianceText->getRenderer()->setTextColor(tgui::Color::Red);
			}
			allegianceText->setPosition("allegianceLabel.right", "allegianceLabel.top");
			m_sideWindow->add(allegianceText);

			std::string growthRateInfo;

			auto growthRateLabel = tgui::Label::create("Growth Rate: " + std::to_string(colony.getGrowthRate(planet.getHabitability(), &growthRateInfo) * 100.0f) + "%");
			growthRateLabel->setPosition("0%", "20%");
			if (growthRateInfo.size() > 0) {
				auto tooltip = tgui::Label::create(growthRateInfo);
				tooltip->getRenderer()->setBackgroundColor(tgui::Color::White);
				tooltip->getRenderer()->setTextColor(tgui::Color::Black);

				growthRateLabel->setToolTip(tooltip);
			}
			m_sideWindow->add(growthRateLabel);

			std::string wealthFlavor;
			float wealth = colony.getWealth();
			if (wealth > 1000.0f) {
				wealthFlavor = "Monstrously Wealthy";
			}
			else if (wealth > 500.0f) {
				wealthFlavor = "Lavish";
			}
			else if (wealth > 250.0f) {
				wealthFlavor = "Well Off";
			}
			else if (wealth > 100.0f) {
				wealthFlavor = "Middling";
			}
			else if (wealth > 50.0f) {
				wealthFlavor = "Poor";
			}
			else if (wealth > 25.0f) {
				wealthFlavor = "Dirt Poor";
			}
			else {
				wealthFlavor = "Destitute";
			}

			auto wealthLabel = tgui::Label::create("Wealth: " + Util::cutOffDecimal(wealth, 2) + " (" + wealthFlavor + ")");
			wealthLabel->setPosition("0%", "30%");
			m_sideWindow->add(wealthLabel);

			std::string stabilityFlavor;
			float stability = colony.getStability();

			if (stability == 1.0f) {
				stabilityFlavor = "Under Control";
			}
			else if (stability > 0.75f) {
				stabilityFlavor = "This Is Fine";
			}
			else if (stability > 0.5f) {
				stabilityFlavor = "Protests";
			}
			else if (stability > 0.25f) {
				stabilityFlavor = "Riots";
			}
			else if (stability > 0.0f) {
				stabilityFlavor = "Civil War";
			}
			else {
				stabilityFlavor = "Government Collapsed";
			}

			auto stabilityLabel = tgui::Label::create("Stability: " + Util::cutOffDecimal(stability, 2) + " (" + stabilityFlavor + ")");
			stabilityLabel->setPosition("0%", "40%");
			m_sideWindow->add(stabilityLabel);
		}
	};
	colonyInfoButton->onClick(openColonyInfo);
	m_buttonPanel->add(colonyInfoButton, "colonyInfoButton");

	// Resources button
	auto resourceInfoButton = GUI::Button::create("Resources");
	resourceInfoButton->setPosition("0%", "16.66% * 4");
	resourceInfoButton->setSize("100%", "16.66%");
	auto openResourceInfo = [this, &gui, &planet, playerFaction]() {
		switchSideWindow("Resources", gui);

		if (m_sideWindow == nullptr) return;

		int playerAllegiance = -1;
		if (playerFaction != nullptr) {
			playerAllegiance = playerFaction->getID();
		}

		auto abundanceLabel = tgui::Label::create();
		abundanceLabel->setPosition("50%", "0%");
		abundanceLabel->setText("");
		m_sideWindow->add(abundanceLabel, "abundanceLabel");

		auto resourceListBox = tgui::ListBox::create();
		resourceListBox->setPosition("2.5%", "5%");
		resourceListBox->setSize("47.5%", "90%");

		if (planet.getColony().getAllegiance() == playerAllegiance) {
			for (auto& resource : planet.getResources()) {
				if (!resource.hidden) {
					resourceListBox->addItem(resource.getName());
				}
			}

			resourceListBox->onItemSelect([this, &planet]() {
				auto listBox = m_sideWindow->get<tgui::ListBox>("resourceListBox");
				auto abundanceLabel = m_sideWindow->get<tgui::Label>("abundanceLabel");
				if (listBox->getSelectedItemIndex() >= 0 && listBox->getSelectedItemIndex() < listBox->getItemCount()) {
					abundanceLabel->setText("Abundance: " + std::to_string(planet.getResources()[listBox->getSelectedItemIndex()].abundance));
				}
				});

			m_sideWindow->add(resourceListBox, "resourceListBox");
		}
		else {
			auto infoLabel = tgui::Label::create("Colonize this planet to discover its resources.");
			infoLabel->setSize("100%", "20%");
			m_sideWindow->add(infoLabel);
		}
	};

	/*if (planet.getColony().getPopulation() > 0) {
		openColonyInfo();
	}
	else {
		openResourceInfo();
	}*/

	resourceInfoButton->onClick(openResourceInfo);
	m_buttonPanel->add(resourceInfoButton);

}

void PlanetGUI::createLawsButton(tgui::Gui& gui, GameState& state, Planet& planet) {
	auto lawsButton = GUI::Button::create("Laws");
	lawsButton->setPosition("0%", "16.66% * 3");
	lawsButton->setSize("100%", "16.66%");
	lawsButton->onPress([this, &gui, &state, &planet]() {
		switchSideWindow("Laws", gui);

		if (m_sideWindow == nullptr) return;

		auto colonyLawLabel = tgui::Label::create("Colonization Legality: ");
		m_sideWindow->add(colonyLawLabel, "colonyLawLabel");

		auto colonyLawComboBox = tgui::ComboBox::create();
		colonyLawComboBox->setPosition("colonyLawLabel.right", "colonyLawLabel.top");
		colonyLawComboBox->setSize("40%", "10%");
		colonyLawComboBox->addItem("Illegal");
		colonyLawComboBox->addItem("Legal");
		colonyLawComboBox->onItemSelect([this, colonyLawComboBox, &planet, &state]() {
			if (colonyLawComboBox->getSelectedItem() == "Illegal") {
				planet.getColony().setFactionColonyLegality(state.getPlayer().getFaction(), false);
			}
			else if (colonyLawComboBox->getSelectedItem() == "Legal") {
				planet.getColony().setFactionColonyLegality(state.getPlayer().getFaction(), true);
			}
			});

		if (planet.getColony().isColonizationLegal(state.getPlayer().getFaction())) {
			colonyLawComboBox->setSelectedItemByIndex(1);
		}
		else {
			colonyLawComboBox->setSelectedItemByIndex(0);
		}

		m_sideWindow->add(colonyLawComboBox);
		});
	m_buttonPanel->add(lawsButton, "lawsButton");
}

void PlanetGUI::createBuildingsButton(tgui::Gui& gui, Planet& planet, Faction* playerFaction) {
	auto buildingsButton = GUI::Button::create("Buildings");
	buildingsButton->setPosition("0%", "16.66% * 2");
	buildingsButton->setSize("100%", "16.66%");
	buildingsButton->onPress(&PlanetGUI::openBuildingsPanel, this, std::ref(gui), std::ref(planet), playerFaction);
	m_buttonPanel->add(buildingsButton);
}

void PlanetGUI::openBuildingsPanel(tgui::Gui& gui, Planet& planet, Faction* playerFaction) {
	switchSideWindow("Buildings", gui);

	if (m_sideWindow == nullptr) return;

	m_sideWindow->setSize(m_sideWindow->getSize() * 2.0f);

	auto buildingsBox = tgui::ListBox::create();
	buildingsBox->setPosition("2.5%", "5%");
	buildingsBox->setSize("47.5%", "90%");
	m_sideWindow->add(buildingsBox, "buildingsBox");

	auto buildings = planet.getColony().getBuildings();

	int buildQueuePos = 1;
	for (ColonyBuilding& building : buildings) {
		if (!building.isBuilt()) {
			buildingsBox->addItem(building.getName() + " (" + std::to_string(buildQueuePos) + ")");
			buildQueuePos++;
		}
		else buildingsBox->addItem(building.getName());
		buildingsBox->setItemData(buildingsBox->getItemCount() - 1, building.getType());
	}

	buildingsBox->onItemSelect([this, buildingsBox, &planet]() {
		auto infoGroup = m_sideWindow->get<tgui::Group>("infoGroup");
		if (infoGroup != nullptr) {
			m_sideWindow->remove(infoGroup);
		}

		if (buildingsBox->getSelectedItemIndex() != -1) {
			std::string type = buildingsBox->getItemData<std::string>(buildingsBox->getSelectedItemIndex());
			auto& allBuildings = planet.getColony().getBuildings();
			for (ColonyBuilding& building : allBuildings) {
				if (building.getType() == type) {
					displayBuildingInfo(building, planet, false);
					break;
				}
			}
		}
		});

	int playerAllegiance = -1;
	if (playerFaction != nullptr) {
		playerAllegiance = playerFaction->getID();
	}

	if (planet.getColony().getAllegiance() == playerAllegiance && playerAllegiance != -1) {
		auto buildButton = GUI::Button::create("Build");
		buildButton->setPosition("buildingsBox.right + 2.5%/2", "85%");
		buildButton->onPress([this, &gui, &planet, playerFaction]() {
			switchSideWindow("Build Colony Buildings", gui);

			if (m_sideWindow == nullptr) return;

			m_sideWindow->setSize(m_sideWindow->getSize() * 2.0f);

			auto backButton = GUI::Button::create("Back");
			backButton->setPosition("50% + 2.5%/2", "90%");
			//backButton->setOrigin({ 1.0, 0.0 });
			backButton->onPress([this, &gui, &planet, playerFaction]() {
				openBuildingsPanel(gui, planet, playerFaction);
			});
			m_sideWindow->add(backButton);

			auto listBox = tgui::ListBox::create();
			listBox->setPosition("2.5%", "5%");
			listBox->setSize("47.5%", "90%");
			m_sideWindow->add(listBox);

			auto buildings = playerFaction->getColonyBuildings();

			// Add buildable buildings to listbox
			for (const ColonyBuilding& building : buildings) {
				if (building.isBuildable(planet.getColony())) {
					listBox->addItem(building.getName());
				}
			}

			listBox->onItemSelect([this, listBox, &planet, playerFaction]() {
				auto infoGroup = m_sideWindow->get<tgui::Group>("infoGroup");
				if (infoGroup != nullptr) {
					m_sideWindow->remove(infoGroup);
				}

				if (listBox->getSelectedItemIndex() != -1) {
					auto allBuildings = playerFaction->getColonyBuildings();
					for (ColonyBuilding& building : allBuildings) {
						if (building.getName() == listBox->getSelectedItem()) {
							displayBuildingInfo(building, planet, true);

							if (!planet.getColony().hasBuildingOfType(building.getType())) {
								auto buildButton = GUI::Button::create("Build");
								buildButton->setPosition("2.5%", "85%");
								m_sideWindow->get<tgui::Group>("infoGroup")->add(buildButton);

								buildButton->onPress([this, buildButton, building, &planet, playerFaction]() {
									if (!planet.getColony().hasBuildingOfType(building.getType())) {
										auto cost = building.getResourceCost(planet);

										if (playerFaction->canSubtractResources(cost)) {
											planet.getColony().addBuilding(building);
											playerFaction->subtractResources(cost);

											m_sideWindow->get<tgui::Group>("infoGroup")->remove(buildButton);
											createBuildStatusLabel(planet, building);
										}
									}
									});
							}
							else {
								createBuildStatusLabel(planet, building);
							}

							break;
						}
					}
				}
				});
			});
		m_sideWindow->add(buildButton);
	}
}

void PlanetGUI::displayBuildingInfo(ColonyBuilding& building, Planet& planet, bool buildInfo) {
	auto infoGroup = m_sideWindow->get<tgui::Group>("infoGroup");
	if (infoGroup != nullptr) {
		m_sideWindow->remove(infoGroup);
	}

	infoGroup = tgui::Group::create();
	infoGroup->setPosition("50%", "0%");
	infoGroup->setSize("50%", "100%");
	m_sideWindow->add(infoGroup, "infoGroup");

	auto nameLabel = tgui::Label::create(building.getName());
	nameLabel->setAutoSize(true);
	nameLabel->getRenderer()->setBorders(1);
	nameLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
	infoGroup->add(nameLabel, "nameLabel");

	auto descriptionLabel = tgui::Label::create(building.getDescription());
	descriptionLabel->setPosition("0%", "nameLabel.bottom");
	descriptionLabel->setSize("100%", "20%");
	descriptionLabel->getRenderer()->setBorders(1);
	descriptionLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
	infoGroup->add(descriptionLabel, "descriptionLabel");

	auto effectsLabel = tgui::Label::create(building.getEffectsString());
	effectsLabel->setPosition("0%", "descriptionLabel.bottom");
	effectsLabel->setSize("100%", "20%");
	effectsLabel->getRenderer()->setBorders(1);
	effectsLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
	infoGroup->add(effectsLabel, "effectsLabel");

	if (!buildInfo) {
		auto statusLabel = tgui::Label::create();
		if (!building.isBuilt()) {
			statusLabel->setText("Status: Under Construction");
		}
		else {
			statusLabel->setText("Status: Operational");
		}
		statusLabel->setPosition("effectsLabel.left", "effectsLabel.bottom");
		statusLabel->setSize("100%", "10%");
		statusLabel->getRenderer()->setBorders(1);
		statusLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
		infoGroup->add(statusLabel, "statusLabel");

		auto buildProgressBar = tgui::ProgressBar::create();
		buildProgressBar->setPosition("statusLabel.left", "statusLabel.bottom");
		infoGroup->add(buildProgressBar, "buildProgressBar");
	}
	else {
		auto cost = building.getResourceCost(planet);
		std::stringstream resourceStr;
		resourceStr << "Build Cost: \n";
		resourceStr << std::fixed << std::setprecision(1);
		for (auto& pair : cost) {
			Resource resource(pair.first);
			resourceStr << resource.getName() << ": " << pair.second << "\n";
		}

		auto costLabel = tgui::Label::create(resourceStr.str());
		costLabel->setPosition("effectsLabel.left", "effectsLabel.bottom");
		costLabel->setSize("100%", "10%");
		costLabel->getRenderer()->setBorders(1);
		costLabel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
		infoGroup->add(costLabel);
	}
}

void PlanetGUI::createBuildStatusLabel(Planet& planet, const ColonyBuilding& building) {
	auto statusLabel = tgui::Label::create();
	statusLabel->setPosition("2.5%", "85%");
	if (planet.getColony().getBuildingOfType(building.getType())->isBuilt()) {
		statusLabel->setText("Built");
	}
	else {
		statusLabel->setText("Under Construction");
	}
	m_sideWindow->get<tgui::Group>("infoGroup")->add(statusLabel);
}

void PlanetGUI::createEventsButton(tgui::Gui& gui, const Planet& planet) {
	auto eventsButton = GUI::Button::create("Events");
	eventsButton->setPosition("0%", "16.66%");
	eventsButton->setSize("100%", "16.66%");
	eventsButton->onPress([this, &gui, &planet]() {
		switchSideWindow("Events", gui);

		if (m_sideWindow == nullptr) return;

		const toml::table& table = TOMLCache::getTable("data/objects/planetevents.toml");

		auto descriptionPanel = tgui::Panel::create();
		descriptionPanel->setPosition("0%", "50%");
		descriptionPanel->setSize("100%", "50%");
		descriptionPanel->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
		m_sideWindow->add(descriptionPanel);

		auto eventsBox = tgui::ListBox::create();
		eventsBox->setSize("100%", "50%");
		m_sideWindow->add(eventsBox);

		for (const Planet::PlanetEvent& ev : planet.getEvents()) {
			eventsBox->addItem(table[ev.type]["name"].value_or(std::string("???")) + " - " + Util::secondsToTime(ev.timeSeconds));
			eventsBox->setItemData(eventsBox->getItemCount() - 1, ev.type);
		}

		eventsBox->onItemSelect([eventsBox, descriptionPanel]() {
			auto descriptionText = descriptionPanel->get<tgui::Label>("descriptionText");
			
			if (eventsBox->getSelectedItemIndex() != -1) {
				const toml::table& table = TOMLCache::getTable("data/objects/planetevents.toml");
				std::string type = eventsBox->getItemData<std::string>(eventsBox->getSelectedItemIndex());

				descriptionText->setText(eventsBox->getSelectedItem() + " - " + table[type]["description"].value_or("???"));
			}
			else {
				descriptionText->setText("");
			}
		});

		auto descriptionText = tgui::Label::create();
		descriptionText->setSize("100%", "100%");
		descriptionPanel->add(descriptionText, "descriptionText");
	});
	m_buttonPanel->add(eventsButton);
}

void PlanetGUI::createTradeButton(tgui::Gui& gui, Planet& planet) {
	auto tradeButton = GUI::Button::create("Trade");
	tradeButton->setPosition("0%", "0%");
	tradeButton->setSize("100%", "16.66%");
	tradeButton->onPress([this, &gui, &planet]() {
		switchSideWindow("Trade", gui);

		if (m_sideWindow == nullptr) return;

		m_sideWindow->setSize(m_sideWindow->getSize().x * 2.0f, m_sideWindow->getSize().y);

		auto goodLabel = tgui::Label::create("Good");
		m_sideWindow->add(goodLabel);

		auto supplyLabel = tgui::Label::create("Supply");
		supplyLabel->setPosition("25%", "0%");
		m_sideWindow->add(supplyLabel);

		auto demandLabel = tgui::Label::create("Demand");
		demandLabel->setPosition("50%", "0%");
		m_sideWindow->add(demandLabel);

		auto priceLabel = tgui::Label::create("Price");
		priceLabel->setPosition("75%", "0%");
		m_sideWindow->add(priceLabel);

		auto infoGroup = tgui::Group::create();
		m_sideWindow->add(infoGroup);

		m_updateFunction = [infoGroup](Planet& planet) {
			int yPos = 10;
			for (auto& good : planet.getColony().getTradeGoods().getGoods()) {
				const toml::table& table = TOMLCache::getTable("data/objects/tradegoods.toml");

				auto nameLabel = infoGroup->get<tgui::Label>(good.first + "name");
				if (nameLabel == nullptr) {
					nameLabel = tgui::Label::create();
					infoGroup->add(nameLabel, good.first + "name");
				}

				nameLabel->setText(table[good.first]["name"].value_or("Unknown"));

				auto supplyLabel = infoGroup->get<tgui::Label>(good.first + "supply");
				auto supplyTrendLabel = infoGroup->get<tgui::Label>(good.first + "supplyTrend");
				auto supplyShortageLabel = infoGroup->get<tgui::Label>(good.first + "supplyShortage");
				if (supplyLabel == nullptr) {
					supplyLabel = tgui::Label::create();
					infoGroup->add(supplyLabel, good.first + "supply");
					
					supplyTrendLabel = tgui::Label::create();
					infoGroup->add(supplyTrendLabel, good.first + "supplyTrend");

					supplyShortageLabel = tgui::Label::create();
					supplyShortageLabel->getRenderer()->setTextColor(tgui::Color::Red);
					infoGroup->add(supplyShortageLabel, good.first + "supplyShortage");
				}
				
				if (good.second.supply < good.second.demand) {
					supplyShortageLabel->setText("SHORTAGE");
				}
				else {
					supplyShortageLabel->setText("");
				}

				updateTrendWidget(supplyTrendLabel, good.second.supplyChange);

				supplyLabel->setText(Util::cutOffDecimal(good.second.supply, 2));

				auto demandLabel = infoGroup->get<tgui::Label>(good.first + "demand");
				auto demandTrendLabel = infoGroup->get<tgui::Label>(good.first + "demandTrend");
				if (demandLabel == nullptr) {
					demandLabel = tgui::Label::create();
					infoGroup->add(demandLabel, good.first + "demand");
					demandTrendLabel = tgui::Label::create();
					infoGroup->add(demandTrendLabel, good.first + "demandTrend");
				}
				
				updateTrendWidget(demandTrendLabel, good.second.demandChange);
				
				demandLabel->setText(Util::cutOffDecimal(good.second.demand, 2));

				auto priceLabel = infoGroup->get<tgui::Label>(good.first + "price");
				auto priceTrendLabel = infoGroup->get<tgui::Label>(good.first + "priceTrend");
				if (priceLabel == nullptr) {
					priceLabel = tgui::Label::create();
					infoGroup->add(priceLabel, good.first + "price");
					priceTrendLabel = tgui::Label::create();
					infoGroup->add(priceTrendLabel, good.first + "priceTrend");
				}

				updateTrendWidget(priceTrendLabel, good.second.priceChange);

				priceLabel->setText("$" + Util::cutOffDecimal(planet.getColony().getTradeGoods().calcPrice(good.first), 2));
				
				tgui::String yPosStr = std::to_string(yPos) + "%";
				bool yChanged = false;
				if (yPosStr != nameLabel->getPositionLayout().y.toString()) yChanged = true;

				if (yChanged) {
					nameLabel->setPosition("0%", yPosStr);
					supplyTrendLabel->setPosition("25%", yPosStr);
					supplyLabel->setPosition("27%", yPosStr);
					supplyShortageLabel->setPosition(tgui::String(good.first + "supply.x + " + good.first + "supply.width"), yPosStr);
					demandTrendLabel->setPosition("50%", yPosStr);
					demandLabel->setPosition("52%", yPosStr);
					priceTrendLabel->setPosition("75%", yPosStr);
					priceLabel->setPosition("77%", yPosStr);
				}

				yPos += 10;
			}
		};
	});
	m_buttonPanel->add(tradeButton);
}

void PlanetGUI::updateTrendWidget(tgui::Label::Ptr& label, float trend) {
	if (trend > 0.0f) {
		label->setText(L"▲");
		label->getRenderer()->setTextColor(tgui::Color::Green);
	}
	else if (trend < 0.0f) {
		label->setText(L"▼");
		label->getRenderer()->setTextColor(tgui::Color::Red);
	}
	else {
		label->setText("~");
		label->getRenderer()->setTextColor(tgui::Color::Yellow);
	}
}

void PlanetGUI::closePanel(tgui::Gui& gui) {
	if (m_planetPanel != nullptr) {
		gui.remove(m_planetPanel);
		gui.remove(m_buttonPanel);
		m_planetPanel = nullptr;
		m_buttonPanel = nullptr;
	}
	if (m_sideWindow != nullptr) {
		gui.remove(m_sideWindow);
		m_sideWindow = nullptr;
	}
}