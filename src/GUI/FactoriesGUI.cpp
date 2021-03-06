#include "gamepch.h"
#include "FactoriesGUI.h"
#include "../Faction.h"
#include "../Sounds.h"
#include "../Constellation.h"
#include "../Mod.h"

void FactoriesGUI::open(tgui::Gui& gui, Faction& faction, Constellation& constellation) {
	m_icon.open(gui, { "0%", "65%" }, { "2.5%", "5%" }, "data/art/factoryicon.png", "Ship Factories");
	m_icon.setLengthScale(2.0f);

	m_icon.panel->onClick([this, &gui, &faction, &constellation]() {
		if (m_window != nullptr) {
			m_window->close();
			m_window = nullptr;
			return;
		}
		
		m_window = GUI::ChildWindow::create("Ship Factories");
		m_window->setOrigin(0.5, 0.5);
		m_window->setPosition("50%", "50%");
		m_window->setSize("75%", "75%");
		m_window->getRenderer()->setOpacity(0.75f);

		gui.add(m_window);

		m_window->onClose([this]() {
			m_window = nullptr;
			m_infoLabel = nullptr;
			m_factoriesToUpdateLabel = nullptr;
		});

		m_shipFactoryPanel = tgui::Panel::create({ "100%", "75%" });
		m_shipFactoryPanel->setPosition("0%", "25%");
		m_shipFactoryPanel->getRenderer()->setTextureBackground(tgui::Texture());
		m_shipFactoryPanel->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
		m_shipFactoryPanel->getRenderer()->setBorders(1);
		m_shipFactoryPanel->getRenderer()->setBorderColor({125, 125, 125});
		m_designListGUI.open(gui, faction, m_shipFactoryPanel.get());
		m_window->add(m_shipFactoryPanel);

		m_slider = tgui::Slider::create();
		m_slider->setPosition("2.5%", "2.5%");
		m_slider->setSize("95%", "2.5%");
		m_slider->onValueChange([]() {
			Sounds::playUISound("data/sound/click.wav");
		});
		m_slider->setStep(1.0f);
		m_slider->setValue(10.0f);
		m_window->add(m_slider);

		m_infoLabel = tgui::Label::create("Test text");
		m_infoLabel->setPosition("2.5%", "5%");
		m_window->add(m_infoLabel);

		m_factoriesToUpdateLabel = tgui::Label::create("More test text");
		m_factoriesToUpdateLabel->setPosition("50%", "5%");
		m_factoriesToUpdateLabel->setOrigin({ 0.5, 0.0 });
		m_window->add(m_factoriesToUpdateLabel);

		auto applyButton = GUI::Button::create("Apply Settings");
		applyButton->setSize("10%", "10%");
		applyButton->setPosition("87.5%", "15%");
		applyButton->setOrigin({ 0.5f, 0.5f });
		m_window->add(applyButton);

		applyButton->onClick([this, &constellation, &faction]() {
			int numShipFactories = countNumShipFactories(constellation, faction.getID());
			int factoriesToUpdate = std::round((m_slider->getValue() / m_slider->getMaximum()) * numShipFactories);
			int factoriesUpdated = 0;

			for (auto& star : constellation.getStars()) {
				if (factoriesUpdated == factoriesToUpdate) {
					break;
				}

				for (auto& building : star->getBuildings()) {
					if (factoriesUpdated == factoriesToUpdate) {
						break;
					}

					if (building->getType() == "SHIP_FACTORY" && building->getAllegiance() == faction.getID()) {
						FactoryMod* mod = building->getMod<FactoryMod>();
						if (mod != nullptr) {
							mod->setShipBuildData(m_designListGUI.shipBuildData);
							factoriesUpdated++;
						}
					}
				}
			}
		});
	});
}

void FactoriesGUI::draw(sf::RenderWindow& window) {
	m_icon.draw(window);
}

void FactoriesGUI::update(Constellation& constellation, Faction* playerFaction) {
	if (m_infoLabel != nullptr && playerFaction != nullptr) {
		int numShipFactories = countNumShipFactories(constellation, playerFaction->getID());

		std::stringstream stream;
		stream << "Ship factory count: " << numShipFactories;

		if (m_infoLabel->getText() != stream.str()) {
			m_infoLabel->setText(stream.str());
		}

		int factoriesToUpdate = std::round((m_slider->getValue() / m_slider->getMaximum()) * numShipFactories);
		std::string factoriesText = "Factories to update: " + std::to_string(factoriesToUpdate);
		
		if (m_factoriesToUpdateLabel->getText() != factoriesText) {
			m_factoriesToUpdateLabel->setText(factoriesText);
		}

		m_slider->setMaximum(numShipFactories);
	}
}

int FactoriesGUI::countNumShipFactories(Constellation& constellation, int allegiance) {
	int numShipFactories = 0;

	for (auto& star : constellation.getStars()) {
		for (auto& building : star->getBuildings()) {
			if (building->getType() == "SHIP_FACTORY" && building->getAllegiance() == allegiance) {
				numShipFactories++;
			}
		}
	}

	return numShipFactories;
}