#include "gamepch.h"
#include "PlanetGUI.h"
#include "../Faction.h"
#include "../Star.h"
#include "../Math.h"

void PlanetGUI::open(tgui::Gui& gui, GameState& state, Faction* playerFaction) {
	if (m_planetPanel != nullptr) {
		gui.remove(m_planetPanel);
		m_planetPanel = nullptr;
	}

	m_planetIconPanel = tgui::Panel::create();
	m_planetIconPanel->setPosition("0%", "85%");
	m_planetIconPanel->setSize("2.5%", "5%");
	m_planetIconPanel->getRenderer()->setOpacity(0.75f);

	m_planetIconPanel->onMouseEnter([this]() {
		m_planetIconPanel->getRenderer()->setBackgroundColor(tgui::Color::White);
		});

	m_planetIconPanel->onMouseLeave([this]() {
		m_planetIconPanel->getRenderer()->setBackgroundColor(tgui::Color(80, 80, 80));
		m_planetIconPanel->getRenderer()->setOpacity(0.75f);
		});

	m_planetIconPanel->onClick([this, &gui, &state, playerFaction]() {
		if (m_planetPanel == nullptr) {
			if (state.getLocalViewStar() != nullptr) {
				m_planetPanel = tgui::Panel::create();
				m_planetPanel->setInheritedOpacity(0.75);
				m_planetPanel->setPosition("2.5%", "61%");
				m_planetPanel->setSize("20%", "29%");
				gui.add(m_planetPanel, "planetPanel");

				m_planetInfoPanel = tgui::Panel::create();
				m_planetInfoPanel->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
				m_planetInfoPanel->getRenderer()->setBorderColor(tgui::Color(100, 100, 100));
				m_planetInfoPanel->getRenderer()->setBorders(1.0f);
				m_planetInfoPanel->setPosition("0%", "20%");
				m_planetInfoPanel->setSize("100%", "80%");
				m_planetPanel->add(m_planetInfoPanel);

				std::vector<Planet>& planets = state.getLocalViewStar()->getPlanets();

				auto planetList = tgui::ComboBox::create();
				planetList->setPosition("25%", "5%");
				planetList->setSize("50%", "10%");
				planetList->onItemSelect([this, planetList, &state, &gui, playerFaction]() {
					setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getSelectedItemIndex());
					});
				m_planetPanel->add(planetList, "planetList");

				// Add planets to dropdown list if star is discovered
				int playerAllegiance = -1;
				if (playerFaction != nullptr) {
					playerAllegiance = playerFaction->getID();
				}

				if (state.getLocalViewStar()->isDiscovered(playerAllegiance)) {
					for (int i = 0; i < planets.size(); i++) {
						planetList->addItem(std::to_string(i + 1) + ": " + planets[i].getTypeString());
					}
				}

				// Set first planet as selected
				if (planetList->getItemCount() > 0) {
					setSelectedPlanet(planetList, state, playerFaction, gui, 0);
				}

				auto backButton = tgui::Button::create("<-");
				backButton->setPosition("5%", "5%");
				backButton->onClick([this, planetList, &state, &gui, playerFaction]() {
					if (planetList->getItemCount() > 1) {
						if (planetList->getSelectedItemIndex() == 0) {
							setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getItemCount() - 1);
						}
						else {
							setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getSelectedItemIndex() - 1);
						}
					}
					});
				m_planetPanel->add(backButton);

				auto forwardButton = tgui::Button::create("->");
				forwardButton->setPosition("84%", "5%");
				forwardButton->onClick([this, planetList, &state, &gui, playerFaction]() {
					if (planetList->getItemCount() > 1) {
						if (planetList->getSelectedItemIndex() == planetList->getItemCount() - 1) {
							setSelectedPlanet(planetList, state, playerFaction, gui, 0);
						}
						else {
							setSelectedPlanet(planetList, state, playerFaction, gui, planetList->getSelectedItemIndex() + 1);
						}
					}
					});
				m_planetPanel->add(forwardButton);
			}
		}
		else {
			gui.remove(m_planetPanel);
			m_planetPanel = nullptr;

			if (m_sideWindow != nullptr) {
				gui.remove(m_sideWindow);
				m_sideWindow = nullptr;
			}
		}
		});

	state.addOnChangeStateCallback([this, &gui]() {
		if (m_planetPanel != nullptr) {
			gui.remove(m_planetPanel);
			m_planetPanel = nullptr;
		}
		if (m_sideWindow != nullptr) {
			gui.remove(m_sideWindow);
			m_sideWindow = nullptr;
		}
		});

	gui.add(m_planetIconPanel);

	auto picture = tgui::Picture::create("data/art/planetsicon.png");
	picture->setSize("100%", "100%");
	m_planetIconPanel->add(picture);
}

void PlanetGUI::setSelectedPlanet(tgui::ComboBox::Ptr planetList, GameState& state, Faction* playerFaction, tgui::Gui& gui, int index) {
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
	planetTemperatureLabel->setText("Temperature: " + std::to_string(planet.getTemperature()));
	planetTemperatureLabel->setPosition("0%", "15%");
	m_planetInfoPanel->add(planetTemperatureLabel);

	auto planetAtmosLabel = tgui::Label::create();
	planetAtmosLabel->setText("Atmosphere: " + std::to_string(planet.getAtmosphericPressure()));
	planetAtmosLabel->setPosition("0%", "25%");
	m_planetInfoPanel->add(planetAtmosLabel);

	auto planetWaterLabel = tgui::Label::create();
	planetWaterLabel->setText("Water: " + std::to_string(planet.getWater()));
	planetWaterLabel->setPosition("0%", "35%");
	m_planetInfoPanel->add(planetWaterLabel);

	auto planetHabitabilityLabel = tgui::Label::create();
	planetHabitabilityLabel->setText("Habitability: " + std::to_string(planet.getHabitability()));
	planetHabitabilityLabel->setPosition("0%", "45%");
	m_planetInfoPanel->add(planetHabitabilityLabel);

	// Buttons

	createColonyAndResourcesButtons(gui, state, planet);

	if (state.getPlayer().getFaction() != -1) {
		createLawsButton(gui, state, planet);
	}

	createBuildingsButton(gui, planet, playerFaction);

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
		}

		if (m_sideWindow != nullptr) {
			if (m_sideWindow->getTitle() == "Buildings") {
				auto buildingsBox = m_sideWindow->get<tgui::ListBox>("buildingsBox");

				if (buildingsBox->getSelectedItemIndex() != -1) {
					Planet& planet = state.getLocalViewStar()->getPlanets()[planetList->getSelectedItemIndex()];

					for (ColonyBuilding& building : planet.getColony().getBuildings()) {
						if (building.getName() == buildingsBox->getSelectedItem()) {
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
		m_sideWindow->setPosition("planetPanel.right", "planetPanel.bottom");
		m_sideWindow->setSize("20%", "29%");
		m_sideWindow->getRenderer()->setOpacity(0.75f);
		m_sideWindow->setPositionLocked(true);
		m_sideWindow->onClose([this]() {
			m_sideWindow = nullptr;
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
}

void PlanetGUI::onEvent(const sf::Event& ev, tgui::Gui& gui, GameState& state, Faction* playerFaction, const sf::RenderWindow& window, Star* currentStar, tgui::Panel::Ptr mainPanel) {
	if (currentStar != nullptr && m_planetIconPanel != nullptr && mainPanel->isFocused()) {
		static sf::Vector2f lastMousePressPos;

		if (ev.type == sf::Event::EventType::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left) {
			sf::Vector2i screenPos = sf::Mouse::getPosition(window);
			sf::Vector2f worldPos = window.mapPixelToCoords(screenPos);

			int i = 0;
			for (Planet& planet : currentStar->getPlanets()) {
				if (Math::distance(worldPos, planet.getPos()) < planet.getRadius() &&
					Math::distance(lastMousePressPos, planet.getPos()) < planet.getRadius()) {
					// Open planet panel
					if (m_planetPanel == nullptr) {
						m_planetIconPanel->onClick.emit(m_planetIconPanel.get(), worldPos);
					}
					setSelectedPlanet(m_planetPanel->get<tgui::ComboBox>("planetList"), state, playerFaction, gui, i);
					break;
				}
				i++;
			}
		}
		else if (ev.type == sf::Event::EventType::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
			lastMousePressPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		}
	}
}

void PlanetGUI::createColonyAndResourcesButtons(tgui::Gui& gui, GameState& state, Planet& planet) {
	// Colony button
	auto colonyInfoButton = tgui::Button::create();
	colonyInfoButton->setPosition("75%", "80%");
	colonyInfoButton->setText("Colony");
	colonyInfoButton->setSize("25%", "10%");
	auto openColonyInfo = [this, &gui, &state, &planet]() {
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
			else {
				allegianceText->setText("Hostile");
				allegianceText->getRenderer()->setTextColor(tgui::Color::Red);
			}
			allegianceText->setPosition("allegianceLabel.right", "allegianceLabel.top");
			m_sideWindow->add(allegianceText);

			auto growthRateLabel = tgui::Label::create("Growth Rate: " + std::to_string(colony.getGrowthRate(planet.getHabitability()) * 100.0f) + "%");
			growthRateLabel->setPosition("0%", "20%");
			m_sideWindow->add(growthRateLabel);
		}
	};
	colonyInfoButton->onClick(openColonyInfo);
	m_planetInfoPanel->add(colonyInfoButton, "colonyInfoButton");

	// Resources button
	auto resourceInfoButton = tgui::Button::create("Resources");
	resourceInfoButton->setPosition("colonyInfoButton.left", "colonyInfoButton.top - 10.0%");
	resourceInfoButton->setTextSize(10);
	resourceInfoButton->setSize("25%", "10%");
	auto openResourceInfo = [this, &gui, &planet]() {
		switchSideWindow("Resources", gui);

		if (m_sideWindow == nullptr) return;

		auto abundanceLabel = tgui::Label::create();
		abundanceLabel->setPosition("50%", "0%");
		abundanceLabel->setText("");
		m_sideWindow->add(abundanceLabel, "abundanceLabel");

		auto resourceListBox = tgui::ListBox::create();
		resourceListBox->setPosition("2.5%", "5%");
		resourceListBox->setSize("47.5%", "90%");

		for (auto& resource : planet.getResources()) {
			resourceListBox->addItem(resource.getName());
		}

		resourceListBox->onItemSelect([this, &planet]() {
			auto listBox = m_sideWindow->get<tgui::ListBox>("resourceListBox");
			auto abundanceLabel = m_sideWindow->get<tgui::Label>("abundanceLabel");
			if (listBox->getSelectedItemIndex() >= 0 && listBox->getSelectedItemIndex() < listBox->getItemCount()) {
				abundanceLabel->setText("Abundance: " + std::to_string(planet.getResources()[listBox->getSelectedItemIndex()].abundance));
			}
			});

		m_sideWindow->add(resourceListBox, "resourceListBox");
	};

	if (planet.getColony().getPopulation() > 0) {
		openColonyInfo();
	}
	else {
		openResourceInfo();
	}

	resourceInfoButton->onClick(openResourceInfo);
	m_planetInfoPanel->add(resourceInfoButton);

}

void PlanetGUI::createLawsButton(tgui::Gui& gui, GameState& state, Planet& planet) {
	auto lawsButton = tgui::Button::create("Laws");
	lawsButton->setPosition("colonyInfoButton.left", "colonyInfoButton.top - 20.0%");
	lawsButton->setSize("25%", "10%");
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
	m_planetInfoPanel->add(lawsButton, "lawsButton");
}

void PlanetGUI::createBuildingsButton(tgui::Gui& gui, Planet& planet, Faction* playerFaction) {
	auto buildingsButton = tgui::Button::create("Buildings");
	buildingsButton->setPosition("colonyInfoButton.left", "colonyInfoButton.top - 30.0%");
	buildingsButton->setSize("25%", "10%");
	buildingsButton->onPress([this, &gui, &planet, playerFaction]() {
		switchSideWindow("Buildings", gui);

		if (m_sideWindow == nullptr) return;

		m_sideWindow->setSize(m_sideWindow->getSize() * 2.0f);

		auto buildingsBox = tgui::ListBox::create();
		buildingsBox->setPosition("2.5%", "5%");
		buildingsBox->setSize("47.5%", "90%");
		m_sideWindow->add(buildingsBox, "buildingsBox");

		auto buildings = planet.getColony().getBuildings();

		for (ColonyBuilding& building : buildings) {
			buildingsBox->addItem(building.getName());
		}

		buildingsBox->onItemSelect([this, buildingsBox, &planet]() {
			auto infoGroup = m_sideWindow->get<tgui::Group>("infoGroup");
			if (infoGroup != nullptr) {
				m_sideWindow->remove(infoGroup);
			}

			if (buildingsBox->getSelectedItemIndex() != -1) {
				auto& allBuildings = planet.getColony().getBuildings();
				for (ColonyBuilding& building : allBuildings) {
					if (building.getName() == buildingsBox->getSelectedItem()) {
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
			auto buildButton = tgui::Button::create("Build");
			buildButton->setPosition("buildingsBox.right + 2.5%", "85%");
			buildButton->onPress([this, &gui, &planet, playerFaction]() {
				switchSideWindow("Build Colony Buildings", gui);

				if (m_sideWindow == nullptr) return;

				m_sideWindow->setSize(m_sideWindow->getSize() * 2.0f);

				auto listBox = tgui::ListBox::create();
				listBox->setPosition("2.5%", "5%");
				listBox->setSize("47.5%", "90%");
				m_sideWindow->add(listBox);

				auto buildings = playerFaction->getColonyBuildings();

				// Add buildable buildings to listbox
				for (const ColonyBuilding& building : buildings) {
					listBox->addItem(building.getName());
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
									auto buildButton = tgui::Button::create("Build");
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
		});
	m_planetInfoPanel->add(buildingsButton);
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