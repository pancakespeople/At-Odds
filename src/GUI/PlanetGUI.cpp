#include "gamepch.h"
#include "PlanetGUI.h"
#include "../Faction.h"
#include "../Star.h"
#include "../Math.h"
#include "../TOMLCache.h"
#include "../Util.h"
#include "../Renderer.h"
#include "../Constellation.h"
#include "../Sounds.h"
#include "../Fonts.h"

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
				m_planetInfoPanel->setPosition("0%", "40%");
				m_planetInfoPanel->setSize("100%", "60%");
				m_planetPanel->add(m_planetInfoPanel);

				auto colonizeButton = GUI::Button::create("Colonize");
				colonizeButton->setOrigin({ 0.5f, 0.5f });
				colonizeButton->setPosition("50%", "25%");
				colonizeButton->setClickSound("data/sound/build.wav");
				m_planetPanel->add(colonizeButton, "colonizeButton");

				auto flagPicture = tgui::Picture::create("data/art/redflag.png");
				flagPicture->setSize("20%", "20%");
				flagPicture->setPosition("25%", "25%");
				flagPicture->setOrigin({ 0.5f, 0.5f });
				m_planetPanel->add(flagPicture, "flagPicture");

				m_buttonPanel = tgui::Panel::create();
				m_buttonPanel->setPosition("planetPanel.right", "planetPanel.y");
				m_buttonPanel->setSize("10%", "planetPanel.height");
				gui.add(m_buttonPanel, "buttonPanel");

				m_buttonPanelLayout = tgui::VerticalLayout::create();
				m_buttonPanel->add(m_buttonPanelLayout);

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

				if (planetList->getItemCount() > 0) {
					// Set first planet as selected
					setSelectedPlanet(planetList, state, playerFaction, gui, 0, constellation);
				}
				else {
					colonizeButton->setVisible(false);
					flagPicture->setVisible(false);
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

				colonizeButton->onClick([this, playerFaction, colonizeButton, flagPicture]() {
					m_currentPlanet->getColony().setFactionColonyLegality(playerFaction->getID(), true);
					colonizeButton->setVisible(false);

					flagPicture->getRenderer()->setTexture("data/art/greenflag.png");
				});
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

	m_buttonPanelLayout->removeAllWidgets();

	if (m_mapInfoPanel != nullptr) {
		gui.remove(m_mapInfoPanel);
		m_mapInfoPanel = nullptr;
	}

	Planet& planet = state.getLocalViewStar()->getPlanets()[index];
	m_currentPlanet = &planet;

	// Labels

	std::stringstream text;
	text << "Type: " << planet.getTypeString() << "\n";
	text << "Temperature: " << Util::cutOffDecimal(planet.getTemperature(), 2) << " (" << planet.getTemperatureText() << ")\n";
	text << "Atmosphere: " << Util::cutOffDecimal(planet.getAtmosphericPressure(), 2) << " (" << planet.getAtmosphereText() << ")\n";
	text << "Water: " << Util::cutOffDecimal(planet.getWater(), 2) << " (" << planet.getWaterText() << ")\n";
	text << "Habitability: " << Util::cutOffDecimal(planet.getHabitability(), 2) << " (" << planet.getHabitabilityText() << ")\n";

	auto planetInfoLabel = tgui::Label::create(text.str());
	planetInfoLabel->setSize("100%", "100%");
	m_planetInfoPanel->add(planetInfoLabel);

	// Buttons

	createColonyAndResourcesButtons(gui, state, planet, playerFaction, constellation);

	/*if (state.getPlayer().getFaction() != -1) {
		createLawsButton(gui, state, planet);
	}*/

	createBuildingsButton(gui, planet, playerFaction);
	createEventsButton(gui, planet);
	createTradeButton(gui, planet);
	createMapButton(gui, playerFaction);

	// Focus camera
	state.getCamera().setPos(planet.getPos());

	auto colonizeButton = m_planetPanel->get<tgui::Button>("colonizeButton");
	auto flagPicture = m_planetPanel->get<tgui::Picture>("flagPicture");

	if (playerFaction != nullptr) {
		if (m_currentPlanet->getColony().isColonizationLegal(playerFaction->getID())) {
			colonizeButton->setVisible(false);
			flagPicture->getRenderer()->setTexture("data/art/greenflag.png");
		}
		else {
			colonizeButton->setVisible(true);
			flagPicture->getRenderer()->setTexture("data/art/redflag.png");
		}
	}
	else {
		colonizeButton->setVisible(false);
		flagPicture->getRenderer()->setTexture("data/art/redflag.png");
	}
}

void PlanetGUI::updateSync(GameState& state) {
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
		m_sideWindow->onClose([this, &gui]() {
			m_sideWindow = nullptr;
			m_updateFunction = nullptr;
			m_planetMapCanvas = nullptr;

			if (m_mapInfoPanel != nullptr) {
				gui.remove(m_mapInfoPanel);
				m_mapInfoPanel = nullptr;
			}
		});
		gui.add(m_sideWindow);
	}
	else {
		if (m_sideWindow->getTitle() != name) {
			m_sideWindow->setTitle(name);
			m_sideWindow->setSize("20%", "29%");
			m_sideWindow->getRenderer()->setOpacity(0.75f);
			m_sideWindow->removeAllWidgets();

			if (m_mapInfoPanel != nullptr) {
				gui.remove(m_mapInfoPanel);
				m_mapInfoPanel = nullptr;
			}
		}
		else {
			m_sideWindow->close();
		}
	}
	m_updateFunction = nullptr;
	m_isPlacingBuilding = false;
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
	colonyInfoButton->setText("Colony");
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
				tooltip->getRenderer()->setBorders(1);
				tooltip->getRenderer()->setBackgroundColor(tgui::Color(55, 55, 55));
				tooltip->getRenderer()->setBorderColor(tgui::Color(125, 125, 125));

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
	m_buttonPanelLayout->add(colonyInfoButton, "colonyInfoButton");

	// Resources button
	auto resourceInfoButton = GUI::Button::create("Resources");
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
	m_buttonPanelLayout->add(resourceInfoButton);

}

void PlanetGUI::createLawsButton(tgui::Gui& gui, GameState& state, Planet& planet) {
	auto lawsButton = GUI::Button::create("Laws");
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
	m_buttonPanelLayout->add(lawsButton, "lawsButton");
}

void PlanetGUI::createBuildingsButton(tgui::Gui& gui, Planet& planet, Faction* playerFaction) {
	auto buildingsButton = GUI::Button::create("Buildings");
	buildingsButton->onPress(&PlanetGUI::openBuildingsPanel, this, std::ref(gui), std::ref(planet), playerFaction);
	m_buttonPanelLayout->add(buildingsButton);
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
				if (building.isBuildable(planet.getColony()) && planet.getColony().getBuildingOfType(building.getType()) == nullptr) {
					listBox->addItem(building.getName());
				}
			}

			listBox->onItemSelect([this, listBox, &planet, &gui, playerFaction]() {
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
								//buildButton->setClickSound("data/sound/build.wav");
								buildButton->setPosition("2.5%", "85%");
								m_sideWindow->get<tgui::Group>("infoGroup")->add(buildButton);

								buildButton->onPress([this, buildButton, building, &planet, &gui, playerFaction]() {
									if (!planet.getColony().hasBuildingOfType(building.getType())) {
										auto cost = building.getResourceCost(planet);

										if (playerFaction->canSubtractResources(cost)) {
											// Switch to map for placement picking
											openMapPanel(gui, playerFaction);

											m_isPlacingBuilding = true;
											m_placingBuildingType = building.getType();
											m_placingBuildingTexturePath = building.getTexturePath();
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

	auto effectsLabel = tgui::Label::create(building.getEffectsString() + building.getExtraInfo());
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
	m_buttonPanelLayout->add(eventsButton);
}

void PlanetGUI::createTradeButton(tgui::Gui& gui, Planet& planet) {
	auto tradeButton = GUI::Button::create("Trade");
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
	m_buttonPanelLayout->add(tradeButton);
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
	if (m_mapInfoPanel != nullptr) {
		gui.remove(m_mapInfoPanel);
		m_mapInfoPanel = nullptr;
	}

	m_planetMapCanvas = nullptr;
	m_currentPlanet = nullptr;
}

void PlanetGUI::createMapButton(tgui::Gui& gui, Faction* playerFaction) {
	auto mapButton = GUI::Button::create("Map");
	m_buttonPanelLayout->add(mapButton);

	mapButton->onPress([this, &gui, playerFaction]() {
		openMapPanel(gui, playerFaction);
	});
}

void PlanetGUI::draw(Renderer& renderer, const sf::RenderWindow& window) {
	if (m_planetMapCanvas != nullptr) {
		m_planetMapCanvas->clear();

		renderer.effects.drawPlanetMap(m_planetMapCanvas.get(), *m_currentPlanet);
		drawGrid(renderer, window);

		m_planetMapCanvas->display();
	}
}

void PlanetGUI::updateTileInfo(sf::Vector2i tilePos) {
	int tilePop = 0;
	bool anomaly = false;

	const Colony& colony = m_currentPlanet->getColony();

	if (colony.isGridGenerated()) {
		const Colony::Tile& tile = colony.getTile(tilePos);
		tilePop = tile.population;
		anomaly = tile.anomaly;
	}

	std::stringstream text;
	text << "Tile Info: \n";
	text << "Coordinates: " << "(" << tilePos.x << ", " << tilePos.y << ")\n";
	text << "Population: " << tilePop << "\n";
	text << "Growth rate: " << std::to_string((colony.getGrowthRate(m_currentPlanet->getHabitability()) + colony.getTileGrowthRate(tilePos)) * 100.0f) << "%\n";

	for (ColonyBuilding& building : m_currentPlanet->getColony().getBuildings()) {
		if (building.getPos() == tilePos) {
			text << "Building: " << building.getName() << "\n";
		}
	}

	auto tileInfoLabel = m_mapInfoPanel->get<tgui::Label>("tileInfoLabel");
	auto anomalyLabel = m_mapInfoPanel->get<tgui::Label>("anomalyLabel");
	auto expeditionButton = m_mapInfoPanel->get<tgui::Button>("expeditionButton");

	if (tileInfoLabel->getText() != text.str()) {
		tileInfoLabel->setText(text.str());
	}

	if (anomaly) {
		int expeditionArrivalTime = -1;

		for (const Colony::Expedition& ex : colony.getExpeditions()) {
			if (ex.tileDestination == tilePos) {
				expeditionArrivalTime = ex.finishTimer;
			}
		}
		
		std::string text = "Anomaly detected";

		if (expeditionArrivalTime != -1) {
			text += "\nExpedition arrival time: " + Util::ticksToTime(expeditionArrivalTime);
			expeditionButton->setVisible(false);
		}
		else {
			expeditionButton->setVisible(true);
		}

		if (anomalyLabel->getText() != text) {
			anomalyLabel->setText(text);
		}
	}
	else {
		if (anomalyLabel->getText() != "") {
			anomalyLabel->setText("");
		}

		expeditionButton->setVisible(false);
	}
}

void PlanetGUI::update(GameState& state) {
	if (m_mapInfoPanel != nullptr) {
		if (m_selectedTile != sf::Vector2i{ -1, -1 }) {
			updateTileInfo(m_selectedTile);
		}
	}

	if (m_planetPanel != nullptr) {
		if (m_currentPlanet != nullptr) {
			auto colonizeButton = m_planetPanel->get<tgui::Button>("colonizeButton");
			auto flagPicture = m_planetPanel->get<tgui::Picture>("flagPicture");

			if (colonizeButton->isVisible() && m_currentPlanet->getColony().getAllegiance() == state.getPlayer().getFaction()) {
				colonizeButton->setVisible(false);
				flagPicture->getRenderer()->setTexture("data/art/greenflag.png");
			}
		}
	}
}

void PlanetGUI::openMapPanel(tgui::Gui& gui, Faction* playerFaction) {
	switchSideWindow("Map", gui);
	if (m_sideWindow == nullptr) return;

	m_sideWindow->setSize("40%", "70.8%");
	m_sideWindow->getRenderer()->setOpacity(1.0f);

	m_planetMapCanvas = tgui::Canvas::create();
	m_planetMapCanvas->getRenderer()->setOpacity(1.0f);
	m_sideWindow->add(m_planetMapCanvas);

	m_mapInfoPanel = tgui::Panel::create();
	m_mapInfoPanel->setOrigin({ 0.0f, 1.0f });
	m_mapInfoPanel->setPosition("planetPanel.left", "planetPanel.top");
	m_mapInfoPanel->setSize("planetPanel.width + buttonPanel.width", m_sideWindow->getSize().y - m_planetPanel->getSize().y);
	m_mapInfoPanel->getRenderer()->setOpacity(0.75f);
	m_mapInfoPanel->getRenderer()->setTextureBackground(tgui::Texture("data/tgui/spacepanelbw.png"));
	gui.add(m_mapInfoPanel);

	auto populationCheckBox = tgui::CheckBox::create("Show population");
	populationCheckBox->setPosition("10%", "10%");
	populationCheckBox->setChecked(m_showPopulation);
	m_mapInfoPanel->add(populationCheckBox, "populationCheckBox");

	auto tileInfoLabel = tgui::Label::create();
	tileInfoLabel->setPosition("10%", "20%");
	m_mapInfoPanel->add(tileInfoLabel, "tileInfoLabel");

	auto anomalyLabel = tgui::Label::create();
	anomalyLabel->setPosition("10%", "50%");
	m_mapInfoPanel->add(anomalyLabel, "anomalyLabel");

	auto expeditionButton = tgui::Button::create("Send expedition");
	expeditionButton->setPosition("10%", "60%");
	expeditionButton->setVisible(false);
	m_mapInfoPanel->add(expeditionButton, "expeditionButton");

	auto expeditionTooltip = tgui::Label::create(
		"Launches an expedition of 500 people from the colony's most populated tile to explore the anomaly. \nRequires there to be a tile of at least 500 population on this planet. \nThe explored tile will gain 500 population when the expedition arrives.");
	expeditionTooltip->getRenderer()->setBorders(1);
	expeditionTooltip->getRenderer()->setBackgroundColor(tgui::Color(55, 55, 55));
	expeditionTooltip->getRenderer()->setBorderColor(tgui::Color(125, 125, 125));
	expeditionButton->setToolTip(expeditionTooltip);

	m_planetMapCanvas->onClick([this, playerFaction](tgui::Vector2f pos) {
		sf::Vector2i gridRectSize = sf::Vector2i(sf::Vector2f(m_planetMapCanvas->getSize())) / Colony::GRID_LENGTH;

		int x = std::min((int)pos.x / gridRectSize.x, Colony::GRID_LENGTH - 1);
		int y = std::min((int)pos.y / gridRectSize.y, Colony::GRID_LENGTH - 1);

		m_selectedTile = { x, y };

		if (m_isPlacingBuilding) {
			ColonyBuilding building(m_placingBuildingType);
			auto cost = building.getResourceCost(*m_currentPlanet);
			
			if (playerFaction->canSubtractResources(cost)) {
				building.setPos(m_selectedTile);
				m_currentPlanet->getColony().addBuilding(building);
				
				playerFaction->subtractResources(cost);
				Sounds::playUISound("data/sound/build.wav");
			}

			m_isPlacingBuilding = false;
		}

		});

	populationCheckBox->onChange([this](bool checked) {
		m_showPopulation = checked;
		});

	expeditionButton->onClick([this]() {
		m_currentPlanet->getColony().sendExpedition(m_selectedTile);
		});
}

void PlanetGUI::drawGrid(Renderer& renderer, const sf::RenderWindow& window) {
	const int gridSize = 8;
	const sf::Vector2f gridRectSize = m_planetMapCanvas->getSize() / gridSize;

	// Draw grid
	sf::RectangleShape gridRect;
	gridRect.setSize(gridRectSize);
	gridRect.setFillColor(sf::Color::Transparent);
	gridRect.setOutlineThickness(1.0f);
	gridRect.setOutlineColor(sf::Color(55, 55, 55));

	sf::RectangleShape itemRect;
	itemRect.setSize(gridRectSize);
	itemRect.setFillColor(sf::Color::Transparent);

	sf::Text text;
	text.setFont(Fonts::getMainFont());
	text.setColor(sf::Color(55, 55, 55));

	const Colony& colony = m_currentPlanet->getColony();

	for (int y = 0; y < gridSize; y++) {
		for (int x = 0; x < gridSize; x++) {
			sf::Vector2f pos(x * gridRectSize.x, y * gridRectSize.y);

			gridRect.setPosition(pos);

			sf::FloatRect relBounds = gridRect.getGlobalBounds();
			relBounds.left += m_planetMapCanvas->getAbsolutePosition().x;
			relBounds.top += m_planetMapCanvas->getAbsolutePosition().y;

			if (relBounds.contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
				// Mouse in box

				gridRect.setFillColor({ 255, 255, 255, 125 });
				gridRect.setTexture(nullptr);
			}
			else {
				if (colony.isGridGenerated()) {
					const Colony::Tile& tile = colony.getTile({ x, y });

					if (tile.anomaly) {
						text.setString("?");
						text.setPosition(pos);
						text.setColor(sf::Color::Red);

						gridRect.setTexture(nullptr);
						gridRect.setFillColor(sf::Color::Transparent);
					}
					else if (tile.population > 0) {
						if (m_showPopulation) {
							text.setString(std::to_string(tile.population));
							text.setPosition(pos);
							text.setColor(sf::Color(55, 55, 55));
						}

						gridRect.setFillColor(sf::Color::White);
						gridRect.setTexture(&TextureCache::getTexture(Colony::getCityTexturePath(tile.population, tile.cityVariant)), true);
					}
					else {
						gridRect.setFillColor(sf::Color::Transparent);
						gridRect.setTexture(nullptr);
					}
				}
				else {
					gridRect.setFillColor(sf::Color::Transparent);
				}
			}

			if (sf::Vector2i{ x, y } == m_selectedTile) {
				gridRect.setOutlineColor(sf::Color::Yellow);
			}
			else {
				gridRect.setOutlineColor(sf::Color(55, 55, 55));
			}

			m_planetMapCanvas->draw(gridRect);

			// Buildings
			itemRect.setPosition(pos);
			itemRect.setTexture(nullptr);
			itemRect.setFillColor(sf::Color::Transparent);

			if (relBounds.contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
				if (m_isPlacingBuilding) {
					if (m_placingBuildingTexturePath != "") {
						itemRect.setTexture(&TextureCache::getTexture(m_placingBuildingTexturePath), true);
						itemRect.setFillColor({0, 255, 0, 125});
					}
				}
			}
			else {
				const ColonyBuilding* building = colony.getBuildingAtTile({ x, y });
				if (building != nullptr) {
					std::string texturePath = building->getTexturePath();
					if (texturePath != "") {
						itemRect.setTexture(&TextureCache::getTexture(texturePath), true);
						itemRect.setFillColor(sf::Color::White);
					}
				}
			}

			m_planetMapCanvas->draw(itemRect);

			// Resources
			if (colony.isGridGenerated()) {
				const Colony::Tile& tile = colony.getTile({ x, y });

				if (!tile.anomaly) {
					switch (tile.tileFlag) {
					case Colony::Tile::TileFlag::COMMON_ORE:
						itemRect.setTexture(&TextureCache::getTexture("data/art/kathiumicon.png"), true);
						itemRect.setFillColor(sf::Color::White);
						itemRect.setSize({ 16, 16 });
						m_planetMapCanvas->draw(itemRect);
						break;
					case Colony::Tile::TileFlag::UNCOMMON_ORE:
						itemRect.setTexture(&TextureCache::getTexture("data/art/oscilliteicon.png"), true);
						itemRect.setFillColor(sf::Color::White);
						itemRect.setSize({ 16, 16 });
						m_planetMapCanvas->draw(itemRect);
						break;
					case Colony::Tile::TileFlag::RARE_ORE:
						itemRect.setTexture(&TextureCache::getTexture("data/art/valkicon.png"), true);
						itemRect.setFillColor(sf::Color::White);
						itemRect.setSize({ 16, 16 });
						m_planetMapCanvas->draw(itemRect);
						break;
					}
				}
			}

			if (text.getString() != "") {
				m_planetMapCanvas->draw(text);
				text.setString("");
			}
		}
	}
}

bool PlanetGUI::isOpen() {
	return m_planetPanel != nullptr;
}