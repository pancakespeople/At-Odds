#include "gamepch.h"
#include "ShipDesigner.h"
#include "../Faction.h"
#include "../Designs.h"

void ShipDesignerGUI::open(tgui::Gui& gui, Faction* playerFaction) {
	m_icon.open(gui, tgui::Layout2d("0%", "80%"), tgui::Layout2d("2.5%", "5%"), "data/art/shipicon.png");

	m_icon.panel->onClick([this, &gui, playerFaction]() {
		if (m_window == nullptr) {
			m_window = tgui::ChildWindow::create("Ship Designer");
			m_window->setOrigin(0.5f, 0.5f);
			m_window->setPosition("50%", "50%");
			m_window->setSize("75%", "75%");
			m_window->getRenderer()->setOpacity(0.75f);
			m_window->onClose([this]() {
				m_window = nullptr;
				});

			auto designLabel = tgui::Label::create("Design");
			designLabel->setOrigin(0.5f, 0.5f);
			designLabel->setPosition("12.5%", "2.5%");
			m_window->add(designLabel);

			auto designListBox = tgui::ListBox::create();
			designListBox->setPosition("2.5%", "5%");
			designListBox->setSize("22.5%", "90%");
			designListBox->onItemSelect([this, designListBox, playerFaction]() {
				auto shipChassisListBox = m_window->get<tgui::ListBox>("shipChassisListBox");
				auto shipWeaponsListBox = m_window->get<tgui::ListBox>("shipWeaponsListBox");
				auto designNameTextBox = m_window->get<tgui::EditBox>("designNameTextBox");

				if (designListBox->getSelectedItemIndex() != -1) {
					DesignerShip ship = playerFaction->getShipDesignByName(designListBox->getSelectedItem().toStdString());

					shipChassisListBox->removeAllItems();
					shipChassisListBox->addItem(ship.chassis.name, ship.chassis.type);
					shipChassisListBox->setSelectedItemByIndex(0);

					shipWeaponsListBox->removeAllItems();
					for (auto& weapon : ship.weapons) {
						shipWeaponsListBox->addItem(weapon.name);
					}
					if (shipWeaponsListBox->getItemCount() > 0) {
						shipWeaponsListBox->setSelectedItemByIndex(shipWeaponsListBox->getItemCount() - 1);
					}

					designNameTextBox->setText(ship.name);
					displayShipInfo(playerFaction);
				}
				});
			m_window->add(designListBox, "designListBox");

			auto chassisLabel = tgui::Label::create("Chassis");
			chassisLabel->setOrigin(0.5f, 0.5f);
			chassisLabel->setPosition("38.75%", "2.5%");
			m_window->add(chassisLabel);

			auto chassisListBox = tgui::ListBox::create();
			chassisListBox->setPosition("27.5%", "5%");
			chassisListBox->setSize("22.5%", "30%");
			m_window->add(chassisListBox, "chassisListBox");

			// Add chassis to list box
			for (auto& chassis : playerFaction->getChassis()) {
				chassisListBox->addItem(chassis.name, chassis.type);
			}

			auto weaponsLabel = tgui::Label::create("Weapons");
			weaponsLabel->setOrigin(0.5f, 0.5f);
			weaponsLabel->setPosition("63.75%", "2.5%");
			m_window->add(weaponsLabel);

			auto weaponsListBox = tgui::ListBox::create();
			weaponsListBox->setPosition("52.5%", "5%");
			weaponsListBox->setSize("22.5%", "30%");
			weaponsListBox->onItemSelect([this, &gui, weaponsListBox, playerFaction]() {
				auto weaponInfoGroup = m_window->get<tgui::Group>("weaponInfoGroup");
				if (weaponInfoGroup != nullptr) {
					m_window->remove(weaponInfoGroup);
				}

				if (weaponsListBox->getSelectedItemIndex() != -1) {
					weaponInfoGroup = tgui::Group::create();
					weaponInfoGroup->setPosition("weaponsListBox.right + 2.5%", "weaponsListBox.top");
					m_window->add(weaponInfoGroup, "weaponInfoGroup");

					DesignerWeapon weapon = playerFaction->getWeaponByName(weaponsListBox->getSelectedItem().toStdString());
					Weapon weaponObj(weapon.type);
					
					auto weaponNameLabel = tgui::Label::create(weapon.name);
					weaponNameLabel->getRenderer()->setTextStyle(tgui::TextStyle::Underlined);
					weaponInfoGroup->add(weaponNameLabel);
					
					auto weaponCapacityLabel = tgui::Label::create();
					weaponCapacityLabel->setPosition("0%", "5%");

					std::stringstream ss;
					ss << "Weapon Capacity: " << std::fixed << std::setprecision(1) << weapon.weaponPoints;

					weaponCapacityLabel->setText(ss.str());
					weaponInfoGroup->add(weaponCapacityLabel);

					auto planetAttackLabel = tgui::Label::create();
					planetAttackLabel->setPosition("0%", "10%");

					if (weaponObj.getProjectile().canOrbitallyBombard() || weaponObj.getProjectile().canInvadePlanets()) {
						planetAttackLabel->setText("Attack Planets: Yes");
					}
					else {
						planetAttackLabel->setText("Attack Planets: No");
					}

					weaponInfoGroup->add(planetAttackLabel);

					GUIUtil::displayResourceCost(weaponInfoGroup, weapon.resourceCost, 15);
				}
			});
			m_window->add(weaponsListBox, "weaponsListBox");

			// Add weapons to list box
			for (auto& weapon : playerFaction->getWeapons()) {
				weaponsListBox->addItem(weapon.name);
			}

			auto shipChassisListBox = tgui::ListBox::create();
			shipChassisListBox->setPosition("27.5%", "50%");
			shipChassisListBox->setSize("22.5%", "5%");
			m_window->add(shipChassisListBox, "shipChassisListBox");

			auto chassisInfoLabel = tgui::Label::create();
			chassisInfoLabel->setPosition("27.5%", "60%");
			chassisInfoLabel->setSize("22.5%", "30%");
			m_window->add(chassisInfoLabel, "chassisInfoLabel");

			auto shipWeaponsListBox = tgui::ListBox::create();
			shipWeaponsListBox->setPosition("52.5%", "50%");
			shipWeaponsListBox->setSize("22.5%", "30%");
			m_window->add(shipWeaponsListBox, "shipWeaponsListBox");

			auto designNameLabel = tgui::Label::create("Design Name: ");
			designNameLabel->setOrigin(0.0f, 0.5f);
			designNameLabel->setPosition("27.5%", "90%");
			m_window->add(designNameLabel, "designNameLabel");

			auto designNameTextBox = tgui::EditBox::create();
			designNameTextBox->setPosition("designNameLabel.right", "designNameLabel.top + designNameLabel.height / 2.0");
			designNameTextBox->setOrigin(0.0f, 0.5f);
			designNameTextBox->setSize("22.5%", "5%");
			m_window->add(designNameTextBox, "designNameTextBox");

			auto designNameSaveButton = tgui::Button::create("Save Design");
			designNameSaveButton->setOrigin(0.0f, 0.5f);
			designNameSaveButton->setPosition("designNameTextBox.right + 1%", "designNameTextBox.top + designNameTextBox.height / 2.0");
			designNameSaveButton->onClick([this, designNameTextBox, shipChassisListBox, shipWeaponsListBox, playerFaction]() {
				if (shipChassisListBox->getItemCount() > 0) {
					if (canChassisFitWeapons(playerFaction)) {
						DesignerShip ship;
						ship.chassis = playerFaction->getChassisByName(shipChassisListBox->getItemByIndex(0).toStdString());
						
						for (tgui::String& weapon : shipWeaponsListBox->getItems()) {
							ship.weapons.push_back(playerFaction->getWeaponByName(weapon.toStdString()));
						}
						
						if (designNameTextBox->getText().size() == 0) {
							ship.name = ship.generateName();
						}
						else {
							ship.name = designNameTextBox->getText().toStdString();
						}
						
						playerFaction->addShipDesign(ship);
						displayShipDesigns(playerFaction);
					}
				}
				});
			m_window->add(designNameSaveButton);

			auto chassisAdderButton = tgui::Button::create("+");
			chassisAdderButton->setOrigin(0.5f, 0.5f);
			chassisAdderButton->setSize("10%", "5%");
			chassisAdderButton->setPosition("chassisListBox.left + chassisListBox.width * 0.25", "42.5%");
			chassisAdderButton->onClick([this, playerFaction, chassisListBox, shipChassisListBox]() {
				if (chassisListBox->getSelectedItemIndex() != -1) {
					if (shipChassisListBox->getItemCount() == 0) {
						shipChassisListBox->addItem(chassisListBox->getSelectedItem(), chassisListBox->getSelectedItemId());
						shipChassisListBox->setSelectedItemByIndex(0);
						displayShipInfo(playerFaction);
					}
				}
				});
			m_window->add(chassisAdderButton, "chassisAdderButton");

			auto chassisRemoverButton = tgui::Button::create("-");
			chassisRemoverButton->setOrigin(0.5f, 0.5f);
			chassisRemoverButton->setPosition("chassisListBox.left + chassisListBox.width * 0.75", "42.5%");
			chassisRemoverButton->setSize("chassisAdderButton.size");
			chassisRemoverButton->onClick([this, playerFaction, shipChassisListBox]() {
				if (shipChassisListBox->getSelectedItemIndex() != -1) {
					shipChassisListBox->removeItemByIndex(shipChassisListBox->getSelectedItemIndex());
					displayShipInfo(playerFaction);
				}
				});
			m_window->add(chassisRemoverButton);

			auto weaponsAdderButton = tgui::Button::create("+");
			weaponsAdderButton->setOrigin(0.5f, 0.5f);
			weaponsAdderButton->setSize("chassisAdderButton.size");
			weaponsAdderButton->setPosition("weaponsListBox.left + weaponsListBox.width * 0.25", "42.5%");
			weaponsAdderButton->onClick([this, playerFaction, weaponsListBox, shipWeaponsListBox, shipChassisListBox]() {
				if (weaponsListBox->getSelectedItemIndex() != -1 && shipChassisListBox->getItemCount() > 0) {
					shipWeaponsListBox->addItem(weaponsListBox->getSelectedItem());
					//if (!canChassisFitWeapons(playerFaction)) {
					//	shipWeaponsListBox->removeItemByIndex(shipWeaponsListBox->getItemCount() - 1);
					//}
					if (shipWeaponsListBox->getItemCount() > 0) {
						shipWeaponsListBox->setSelectedItemByIndex(shipWeaponsListBox->getItemCount() - 1);
					}
					displayShipInfo(playerFaction);
				}
				});
			m_window->add(weaponsAdderButton);

			auto weaponsRemoverButton = tgui::Button::create("-");
			weaponsRemoverButton->setOrigin(0.5f, 0.5f);
			weaponsRemoverButton->setPosition("weaponsListBox.left + weaponsListBox.width * 0.75", "42.5%");
			weaponsRemoverButton->setSize("chassisAdderButton.size");
			weaponsRemoverButton->onClick([this, playerFaction, shipWeaponsListBox]() {
				if (shipWeaponsListBox->getSelectedItemIndex() != -1) {
					shipWeaponsListBox->removeItemByIndex(shipWeaponsListBox->getSelectedItemIndex());
					if (shipWeaponsListBox->getItemCount() > 0) {
						shipWeaponsListBox->setSelectedItemByIndex(shipWeaponsListBox->getItemCount() - 1);
					}
					displayShipInfo(playerFaction);
				}
				});
			m_window->add(weaponsRemoverButton);

			shipChassisListBox->onItemSelect([this, shipChassisListBox]() {
				displayChassisInfo(shipChassisListBox->getSelectedItemId().toStdString());
			});

			displayShipDesigns(playerFaction);

			gui.add(m_window);
		}
		else {
			gui.remove(m_window);
			m_window = nullptr;
		}
		});
}

void ShipDesignerGUI::displayShipInfo(Faction* playerFaction) {
	auto shipChassisListBox = m_window->get<tgui::ListBox>("shipChassisListBox");
	auto shipWeaponsListBox = m_window->get<tgui::ListBox>("shipWeaponsListBox");
	auto shipInfoGroup = m_window->get<tgui::Group>("shipInfoGroup");

	if (shipChassisListBox->getItemCount() > 0) {
		tgui::String chassisName = shipChassisListBox->getItemByIndex(0);
		std::unordered_map<std::string, float> totalResourceCost;

		DesignerChassis chassis = playerFaction->getChassisByName(chassisName.toStdString());
		for (auto& resource : chassis.resourceCost) {
			totalResourceCost[resource.first] += resource.second;
		}

		float totalWeaponPoints = 0.0f;

		for (tgui::String& str : shipWeaponsListBox->getItems()) {
			DesignerWeapon weapon = playerFaction->getWeaponByName(str.toStdString());
			totalWeaponPoints += weapon.weaponPoints;
			for (auto& resource : weapon.resourceCost) {
				totalResourceCost[resource.first] += resource.second;
			}
		}

		if (shipInfoGroup != nullptr) {
			m_window->remove(shipInfoGroup);
		}

		shipInfoGroup = tgui::Group::create();
		shipInfoGroup->setPosition("shipWeaponsListBox.right + 2.5%", "shipWeaponsListBox.top");
		shipInfoGroup->setSize("22.5%", "90%");
		m_window->add(shipInfoGroup, "shipInfoGroup");

		auto spaceshipLabel = tgui::Label::create("Spaceship");
		spaceshipLabel->getRenderer()->setTextStyle(tgui::TextStyle::Underlined);
		spaceshipLabel->setPosition("0%", "0%");
		shipInfoGroup->add(spaceshipLabel);

		// Use stringstream to set the decimals properly
		std::stringstream ss;
		ss << "Weapon Capacity: " << std::fixed << std::setprecision(1) << totalWeaponPoints << "/" << chassis.maxWeaponCapacity;

		auto capacityLabel = tgui::Label::create(ss.str());
		capacityLabel->setPosition("0%", "5%");
		//tgui::ToolTip::setInitialDelay(0);
		//capacityLabel->setToolTip(tgui::Label::create("Weapon Capacity"));*/
		shipInfoGroup->add(capacityLabel, "capacityLabel");

		GUIUtil::displayResourceCost(shipInfoGroup, totalResourceCost, 10);
	}
	else {
		if (shipInfoGroup != nullptr) {
			m_window->remove(shipInfoGroup);
		}
	}
}

bool ShipDesignerGUI::canChassisFitWeapons(Faction* playerFaction) {
	auto shipChassisListBox = m_window->get<tgui::ListBox>("shipChassisListBox");
	auto shipWeaponsListBox = m_window->get<tgui::ListBox>("shipWeaponsListBox");

	DesignerChassis chassis = playerFaction->getChassisByName(shipChassisListBox->getItemByIndex(0).toStdString());

	float total = 0.0f;
	for (tgui::String& weaponString : shipWeaponsListBox->getItems()) {
		DesignerWeapon weapon = playerFaction->getWeaponByName(weaponString.toStdString());
		total += weapon.weaponPoints;
	}

	if (total <= chassis.maxWeaponCapacity) return true;
	else return false;
}

void ShipDesignerGUI::displayShipDesigns(Faction* playerFaction) {
	auto designListBox = m_window->get<tgui::ListBox>("designListBox");
	designListBox->removeAllItems();

	for (DesignerShip& ship : playerFaction->getShipDesigns()) {
		designListBox->addItem(ship.name);
	}
}

void ShipDesignerGUI::displayChassisInfo(const std::string& chassisType) {
	auto chassisInfoLabel = m_window->get<tgui::Label>("chassisInfoLabel");

	if (chassisType != "") {
		const toml::table& table = TOMLCache::getTable("data/objects/spaceships.toml");

		DesignerChassis chassis(chassisType);

		std::stringstream ss;
		ss << table[chassisType]["name"].value_or("Unknown") << " Chassis\n";
		ss << "Type: " << table[chassisType]["type"].value_or("N/A") << "\n";
		ss << "Cost: " << Resource::getResourceString(chassis.resourceCost) << "\n";
		ss << "Weapon Capacity: " << table[chassisType]["maxWeaponCapacity"].value_or(0.0f) << "\n";
		ss << "Mass: " << table[chassisType]["mass"].value_or(0.0f) << " kg\n";
		ss << "Health: " << table[chassisType]["health"].value_or(0.0f) << "\n";
		ss << table[chassisType]["extraInfo"].value_or("") << "\n";

		chassisInfoLabel->setText(ss.str());
	}
	else {
		chassisInfoLabel->setText("");
	}
}