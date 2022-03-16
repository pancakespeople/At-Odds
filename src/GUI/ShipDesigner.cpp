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
						shipWeaponsListBox->addItem(weapon.getFullName(), weapon.type);
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

			auto chassisComboBox = tgui::ComboBox::create();
			chassisComboBox->setPosition("27.5%", "5%");
			chassisComboBox->setSize("22.5%", "5%");
			chassisComboBox->setMaximumItems(10);
			m_window->add(chassisComboBox, "chassisComboBox");

			// Add chassis to combo box
			for (auto& chassis : playerFaction->getChassis()) {
				chassisComboBox->addItem(chassis.name, chassis.type);
			}

			chassisComboBox->onItemSelect([this, chassisComboBox]() {
				displayChassisInfo(chassisComboBox->getSelectedItemId().toStdString());
			});

			auto weaponsLabel = tgui::Label::create("Weapons");
			weaponsLabel->setOrigin(0.5f, 0.5f);
			weaponsLabel->setPosition("63.75%", "2.5%");
			m_window->add(weaponsLabel);

			auto weaponsComboBox = tgui::ComboBox::create();
			weaponsComboBox->setPosition("52.5%", "5%");
			weaponsComboBox->setSize("22.5%", "5%");
			weaponsComboBox->onItemSelect([this, &gui, weaponsComboBox, playerFaction]() {
				displayWeaponInfo(weaponsComboBox->getSelectedItemId().toStdString());
			});
			m_window->add(weaponsComboBox, "weaponsComboBox");

			// Add weapons to combo box
			for (auto& weapon : playerFaction->getWeapons()) {
				weaponsComboBox->addItem(weapon.getFullName(), weapon.type);
			}

			auto weaponInfoLabel = tgui::Label::create();
			weaponInfoLabel->setPosition("weaponsComboBox.left", "weaponsComboBox.bottom + 2.5%");
			weaponInfoLabel->setSize("weaponsComboBox.width", "30%");
			m_window->add(weaponInfoLabel, "weaponInfoLabel");

			auto shipChassisListBox = tgui::ListBox::create();
			shipChassisListBox->setPosition("27.5%", "50%");
			shipChassisListBox->setSize("22.5%", "30%");
			m_window->add(shipChassisListBox, "shipChassisListBox");

			auto chassisInfoLabel = tgui::Label::create();
			chassisInfoLabel->setPosition("chassisComboBox.left", "chassisComboBox.bottom + 2.5%");
			chassisInfoLabel->setSize("chassisComboBox.width", "30%");
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

			auto designErrorMessage = tgui::Label::create();
			designErrorMessage->setPosition("designNameLabel.left", "designNameLabel.bottom");
			designErrorMessage->getRenderer()->setTextColor(tgui::Color::Red);
			m_window->add(designErrorMessage);

			auto designNameSaveButton = GUI::Button::create("Save Design");
			designNameSaveButton->setOrigin(0.0f, 0.5f);
			designNameSaveButton->setPosition("designNameTextBox.right + 1%", "designNameTextBox.top + designNameTextBox.height / 2.0");
			designNameSaveButton->onClick([this, designNameTextBox, shipChassisListBox, shipWeaponsListBox, designErrorMessage, playerFaction]() {
				if (shipChassisListBox->getItemCount() > 0) {
					
					std::string errMsg;

					if (canChassisFitWeapons(playerFaction, errMsg)) {
						DesignerShip ship;
						ship.chassis = playerFaction->getChassisByName(shipChassisListBox->getItemByIndex(0).toStdString());
						
						for (tgui::String& weapon : shipWeaponsListBox->getItemIds()) {
							ship.weapons.push_back(playerFaction->getWeapon(weapon.toStdString()));
						}
						
						if (designNameTextBox->getText().size() == 0) {
							ship.name = ship.generateName();
						}
						else {
							ship.name = designNameTextBox->getText().toStdString();
						}
						
						playerFaction->addShipDesign(ship);
						displayShipDesigns(playerFaction);

						designErrorMessage->setText("");
					}
					else {
						designErrorMessage->setText("Failed to save design: " + errMsg);
					}
				}
				else {
					designErrorMessage->setText("Failed to save design: Missing chassis");
				}
				});
			m_window->add(designNameSaveButton);

			auto chassisAdderButton = GUI::Button::create("Add Chassis");
			chassisAdderButton->setOrigin(0.5f, 0.5f);
			chassisAdderButton->setSize("10%", "5%");
			chassisAdderButton->setPosition("chassisComboBox.left + chassisComboBox.width * 0.25", "42.5%");
			chassisAdderButton->onClick([this, playerFaction, chassisComboBox, shipChassisListBox]() {
				if (chassisComboBox->getSelectedItemIndex() != -1) {
					if (shipChassisListBox->getItemCount() == 0) {
						shipChassisListBox->addItem(chassisComboBox->getSelectedItem(), chassisComboBox->getSelectedItemId());
						shipChassisListBox->setSelectedItemByIndex(0);
						displayShipInfo(playerFaction);
					}
				}
				});
			m_window->add(chassisAdderButton, "chassisAdderButton");

			auto chassisRemoverButton = GUI::Button::create("Remove Chassis");
			chassisRemoverButton->setOrigin(0.5f, 0.5f);
			chassisRemoverButton->setPosition("chassisComboBox.left + chassisComboBox.width * 0.75", "42.5%");
			chassisRemoverButton->setSize("chassisAdderButton.size");
			chassisRemoverButton->setTextSize(12);
			chassisRemoverButton->onClick([this, playerFaction, shipChassisListBox]() {
				if (shipChassisListBox->getSelectedItemIndex() != -1) {
					shipChassisListBox->removeItemByIndex(shipChassisListBox->getSelectedItemIndex());
					displayShipInfo(playerFaction);
				}
				});
			m_window->add(chassisRemoverButton);

			auto weaponsAdderButton = GUI::Button::create("Add Weapon");
			weaponsAdderButton->setOrigin(0.5f, 0.5f);
			weaponsAdderButton->setSize("chassisAdderButton.size");
			weaponsAdderButton->setPosition("weaponsComboBox.left + weaponsComboBox.width * 0.25", "42.5%");
			weaponsAdderButton->onClick([this, playerFaction, weaponsComboBox, shipWeaponsListBox, shipChassisListBox]() {
				if (weaponsComboBox->getSelectedItemIndex() != -1 && shipChassisListBox->getItemCount() > 0) {
					shipWeaponsListBox->addItem(weaponsComboBox->getSelectedItem(), weaponsComboBox->getSelectedItemId());
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

			auto weaponsRemoverButton = GUI::Button::create("Remove Weapon");
			weaponsRemoverButton->setOrigin(0.5f, 0.5f);
			weaponsRemoverButton->setPosition("weaponsComboBox.left + weaponsComboBox.width * 0.75", "42.5%");
			weaponsRemoverButton->setSize("chassisAdderButton.size");
			weaponsRemoverButton->setTextSize(12);
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

			auto spaceshipLabel = tgui::Label::create("Spaceship");
			spaceshipLabel->setOrigin(0.5, 0.5);
			spaceshipLabel->setPosition("86.75%", "2.5%");
			m_window->add(spaceshipLabel);

			auto shipInfoLabel = tgui::Label::create();
			shipInfoLabel->setPosition("weaponsComboBox.right + 2.5%", "weaponsComboBox.top");
			shipInfoLabel->setSize("22.5%", "30%");
			m_window->add(shipInfoLabel, "shipInfoLabel");

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
	auto shipInfoLabel = m_window->get<tgui::Label>("shipInfoLabel");

	if (shipChassisListBox->getItemCount() > 0) {
		tgui::String chassisName = shipChassisListBox->getItemByIndex(0);
		std::unordered_map<std::string, float> totalResourceCost;

		DesignerChassis chassis = playerFaction->getChassisByName(chassisName.toStdString());
		for (auto& resource : chassis.resourceCost) {
			totalResourceCost[resource.first] += resource.second;
		}

		float totalWeaponPoints = 0.0f;

		for (tgui::String& str : shipWeaponsListBox->getItemIds()) {
			DesignerWeapon weapon = playerFaction->getWeapon(str.toStdString());
			totalWeaponPoints += weapon.weaponPoints;
			for (auto& resource : weapon.resourceCost) {
				totalResourceCost[resource.first] += resource.second;
			}
		}

		std::stringstream ss;
		ss << std::fixed << std::setprecision(1);
		ss << "Weapon Capacity: " << totalWeaponPoints << "/" << chassis.maxWeaponCapacity << "\n";
		ss << "Cost: " << Resource::getResourceString(totalResourceCost);

		shipInfoLabel->setText(ss.str());
	}
	else {
		shipInfoLabel->setText("");
	}
}

bool ShipDesignerGUI::canChassisFitWeapons(Faction* playerFaction, std::string& errMsg) {
	auto shipChassisListBox = m_window->get<tgui::ListBox>("shipChassisListBox");
	auto shipWeaponsListBox = m_window->get<tgui::ListBox>("shipWeaponsListBox");

	DesignerChassis chassis = playerFaction->getChassisByName(shipChassisListBox->getItemByIndex(0).toStdString());
	
	float total = 0.0f;
	for (tgui::String& weaponString : shipWeaponsListBox->getItemIds()) {
		DesignerWeapon weapon = playerFaction->getWeapon(weaponString.toStdString());

		if ((weapon.miningWeapon && !chassis.miningChassis) || (!weapon.miningWeapon && chassis.miningChassis)) {
			errMsg = "The " + chassis.name + " chassis cannot fit a " + weapon.name;
			return false;
		}
		if (weapon.constructionWeapon && !chassis.constructionChassis || (!weapon.constructionWeapon && chassis.constructionChassis)) {
			errMsg = "The " + chassis.name + " chassis cannot fit a " + weapon.name;
			return false;
		}
		total += weapon.weaponPoints;
	}

	if (total <= chassis.maxWeaponCapacity) return true;
	else {
		errMsg = "The design exceeds the maximum weapon capacity of the chassis";
		return false;
	}
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

void ShipDesignerGUI::displayWeaponInfo(const std::string& weaponType) {
	auto weaponInfoGroup = m_window->get<tgui::Label>("weaponInfoLabel");
	
	if (weaponType != "") {

		DesignerWeapon weapon(weaponType);
		Weapon weaponObj(weaponType);
		
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1);
		ss << weapon.getFullName() << " Weapon\n";
		ss << "Weapon Capacity: " << weapon.weaponPoints << "\n";
		ss << "Attack Planets: " << (weaponObj.getProjectile().canOrbitallyBombard() || weaponObj.getProjectile().canInvadePlanets() ? "Yes" : "No") << "\n";
		ss << "Cost: " << Resource::getResourceString(weapon.resourceCost);

		weaponInfoGroup->setText(ss.str());
	}
	else {
		weaponInfoGroup->setText("");
	}
}