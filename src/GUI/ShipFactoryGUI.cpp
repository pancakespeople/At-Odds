#include "gamepch.h"
#include "ShipFactoryGUI.h"
#include "../Faction.h"

void ShipFactoryGUI::open(tgui::Gui& gui, Faction& faction, tgui::Group* group) {
	m_designsListBox = tgui::ListBox::create();
	m_designsListBox->setPosition("2.5%", "5%");
	m_designsListBox->setSize("33% - 2.5%", "50% - 2.5%");
	
	m_designsListBox->onItemSelect([this, &faction]() {
		if (m_designsListBox->getSelectedItemIndex() != -1) {
			DesignerShip ship = faction.getShipDesignByName(m_designsListBox->getSelectedItem().toStdString());
			int buildIndex = -1;

			// Find build index
			for (int i = 0; i < m_shipBuildData.size(); i++) {
				if (m_shipBuildData[i].shipName == ship.name) buildIndex = i;
			}

			if (ship.name == "") return;

			auto totalResourceCost = ship.getTotalResourceCost();
			m_shipWidgets->removeAllWidgets();

			auto shipInfoGroup = tgui::Group::create();
			shipInfoGroup->setPosition("parent.designsListBox.right + 2.5%", "10%");
			shipInfoGroup->setSize("33% - 2.5%", "90% - 2.5%");
			m_shipWidgets->add(shipInfoGroup, "shipInfoGroup");

			GUIUtil::displayResourceCost(shipInfoGroup, totalResourceCost, 0, 10);

			auto buildCheckbox = tgui::CheckBox::create("Build");
			buildCheckbox->setPosition("shipInfoGroup.right + 2.5%", "shipInfoGroup.top");
			m_shipWidgets->add(buildCheckbox, "buildCheckbox");

			auto continuousCheckbox = tgui::CheckBox::create("Continuous");
			continuousCheckbox->setPosition("shipInfoGroup.right + 2.5%", "shipInfoGroup.top + 15%");
			m_shipWidgets->add(continuousCheckbox);

			auto amountEditBox = tgui::EditBox::create();
			amountEditBox->setPosition("shipInfoGroup.right + 2.5%", "shipInfoGroup.top + 30%");
			amountEditBox->setInputValidator("[0-9]+");
			amountEditBox->setSize("10%", "10%");
			amountEditBox->setMaximumCharacters(2);
			amountEditBox->setText("1");
			m_shipWidgets->add(amountEditBox, "amountEditBox");

			auto amountLabel = tgui::Label::create("Amount");
			amountLabel->setPosition("amountEditBox.right + 1%", "amountEditBox.top");
			m_shipWidgets->add(amountLabel);

			auto moveDownButton = GUI::Button::create("Queue Down");
			moveDownButton->setPosition("33% + 2.5%", "50%");
			moveDownButton->setSize("15%", "5%");
			m_shipWidgets->add(moveDownButton, "moveUpButton");

			auto moveUpButton = GUI::Button::create("Queue Up");
			moveUpButton->setPosition("33% + 2.5%", "moveUpButton.top - moveUpButton.height");
			moveUpButton->setSize("15%", "5%");
			m_shipWidgets->add(moveUpButton);

			// Create ship build data if it doesnt exist or init the widget values with the data if it does
			if (buildIndex == -1) {
				FactoryMod::ShipBuildData data(ship.name);
				m_shipBuildData.push_back(data);

				buildIndex = m_shipBuildData.size() - 1;
			}
			else {
				buildCheckbox->setChecked(m_shipBuildData[buildIndex].build);
				continuousCheckbox->setChecked(m_shipBuildData[buildIndex].continuous);
				amountEditBox->setText(std::to_string(m_shipBuildData[buildIndex].amount));
			}

			// Set selected design
			for (auto& data : m_shipBuildData) {
				if (data.shipName == ship.name) {
					data.selected = true;
				}
				else {
					data.selected = false;
				}
			}

			m_buildProgressBar = tgui::ProgressBar::create();
			m_buildProgressBar->setPosition("parent.designsListBox.right + 2.5%", "80%");
			m_buildProgressBar->setSize("66% - 5%", "10%");
			m_buildProgressBar->setValue(m_shipBuildData[buildIndex].progressPercent);
			m_shipWidgets->add(m_buildProgressBar);

			// Callbacks
			amountEditBox->onUnfocus([this, ship, buildIndex, amountEditBox]() {
				if (amountEditBox->getText().size() > 0) {
					m_shipBuildData[buildIndex].amount = amountEditBox->getText().toInt();
				}
				});

			continuousCheckbox->onChange([this, ship, buildIndex, continuousCheckbox]() {
				m_shipBuildData[buildIndex].continuous = continuousCheckbox->isChecked();
				});

			buildCheckbox->onChange([this, ship, buildIndex, buildCheckbox]() {
				m_shipBuildData[buildIndex].build = buildCheckbox->isChecked();
				updateDesignsListBox(buildIndex);
				});

			moveUpButton->onClick([this, buildIndex]() {
				if (buildIndex != 0) {
					// Swap elements
					FactoryMod::ShipBuildData before = m_shipBuildData[static_cast<size_t>(buildIndex) - 1];
					m_shipBuildData[static_cast<size_t>(buildIndex) - 1] = m_shipBuildData[buildIndex];
					m_shipBuildData[buildIndex] = before;
					updateDesignsListBox(buildIndex - 1);
				}
				});

			moveDownButton->onClick([this, buildIndex]() {
				if (buildIndex != m_shipBuildData.size() - 1) {
					// Swap elements
					FactoryMod::ShipBuildData after = m_shipBuildData[static_cast<size_t>(buildIndex) + 1];
					m_shipBuildData[static_cast<size_t>(buildIndex) + 1] = m_shipBuildData[buildIndex];
					m_shipBuildData[buildIndex] = after;
					updateDesignsListBox(buildIndex + 1);
				}
				});
		}
		else {
			m_shipWidgets->removeAllWidgets();
			m_buildProgressBar = nullptr;
		}
		});
	group->add(m_designsListBox, "designsListBox");

	auto designsLabel = tgui::Label::create("Design Queue");
	designsLabel->setOrigin(0.5f, 0.0f);
	designsLabel->setPosition("designsListBox.width / 2 + designsListBox.left", "0%");
	group->add(designsLabel);

	updateDesigns(&faction);
	updateDesignsListBox(-1);

	m_shipWidgets = tgui::Group::create();
	group->add(m_shipWidgets, "shipWidgets");
}

void ShipFactoryGUI::updateDesigns(Faction* faction) {
	for (DesignerShip ship : faction->getShipDesigns()) {
		bool found = false;
		for (const FactoryMod::ShipBuildData& data : m_shipBuildData) {
			if (data.shipName == ship.name) {
				found = true;
				break;
			}
		}

		if (!found) {
			FactoryMod::ShipBuildData data(ship.name);
			m_shipBuildData.push_back(data);
		}
	}
}

void ShipFactoryGUI::updateDesignsListBox(int selectedIndex) {
	if (m_designsListBox != nullptr) {
		m_designsListBox->removeAllItems();
		for (const FactoryMod::ShipBuildData& build : m_shipBuildData) {
			m_designsListBox->addItem(build.shipName);
		}

		if (selectedIndex != -1) {
			m_designsListBox->setSelectedItemByIndex(selectedIndex);
		}
	}
}