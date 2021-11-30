#include "gamepch.h"
#include "TechGUI.h"
#include "../Faction.h"
#include "../Util.h"

void TechGUI::open(tgui::Gui& gui, Faction* playerFaction) {
	m_icon.open(gui, tgui::Layout2d("0%", "70%"), tgui::Layout2d("2.5%", "5%"), "data/art/techicon.png");

	m_icon.panel->onClick([this, &gui, playerFaction]() {
		if (m_window != nullptr) {
			m_window->close();
			m_window = nullptr;
			return;
		}
		
		m_window = tgui::ChildWindow::create("Tech");
		m_window->setOrigin(0.5, 0.5);
		m_window->setPosition("50%", "50%");
		m_window->setSize("75%", "75%");
		m_window->getRenderer()->setOpacity(0.75f);
		gui.add(m_window);

		m_window->onClose([this]() {
			m_window = nullptr;
		});

		auto tabs = tgui::Tabs::create();
		tabs->setPosition("2.5%", "0%");
		tabs->setSize("50% - 2.5%", "5%");
		tabs->add("Colony", true);
		tabs->add("Spaceship", false);
		tabs->add("Weapons", false);
		m_window->add(tabs);

		auto researchableTechs = tgui::ListBox::create();
		researchableTechs->setPosition("2.5%", "5%");
		researchableTechs->setSize("50% - 2.5%", "50% - 2.5%");
		m_window->add(researchableTechs, "researchableTechs");

		/*auto researchLabel = tgui::Label::create("Available Research");
		researchLabel->setOrigin(0.5, 0.0);
		researchLabel->setPosition("researchableTechs.left + researchableTechs.width / 2", "0%");
		m_window->add(researchLabel);*/

		m_techQueue = tgui::ListBox::create();
		m_techQueue->setPosition("50%", "5%");
		m_techQueue->setSize("50% - 2.5%", "50% - 2.5%");
		m_window->add(m_techQueue, "techQueue");

		auto queueLabel = tgui::Label::create("Queue");
		queueLabel->setOrigin(0.5, 0.0);
		queueLabel->setPosition("techQueue.left + techQueue.width / 2", "0%");
		m_window->add(queueLabel);

		auto researchButton = tgui::Button::create("Research");
		researchButton->setPosition("2.5%", "52.5%");
		researchButton->setVisible(false);
		m_window->add(researchButton);

		m_description = tgui::Label::create();
		m_description->setPosition("2.5%", "62.5%");
		m_window->add(m_description);

		m_progressBar = tgui::ProgressBar::create();
		m_progressBar->setPosition("2.5%", "75%");
		m_progressBar->setSize("95%", "10%");
		m_progressBar->setVisible(false);
		m_window->add(m_progressBar);

		for (const Tech& tech : playerFaction->getTechs()) {
			if (!tech.isResearched() && !tech.isResearching() && tech.getCategory() == tabs->getSelected()) {
				researchableTechs->addItem(tech.getName(), tech.getType());
			}
			else if (tech.isResearching()) {
				m_techQueue->addItem(tech.getName(), tech.getType());
			}
		}

		researchableTechs->onItemSelect([researchableTechs, this, researchButton, playerFaction](int index) {
			if (index != -1) {
				const Tech* tech = playerFaction->getTech(researchableTechs->getSelectedItemId().toStdString());

				if (tech != nullptr) {
					std::string techType = tech->getType();
					m_techQueue->deselectItem();

					m_description->setText(tech->getExtendedDescription(playerFaction));

					researchButton->setVisible(true);
					researchButton->onClick.disconnectAll();
					researchButton->onClick([techType, researchableTechs, this, playerFaction]() {
						const Tech* tech = playerFaction->getTech(techType);

						if (tech != nullptr) {
							researchableTechs->removeItem(researchableTechs->getSelectedItem());
							m_techQueue->addItem(tech->getName(), tech->getType());
							
							// Invalidates pointers
							playerFaction->setResearchingTech(tech->getType(), true);
						}
					});
				}
			}
			else {
				m_description->setText("");
				researchButton->setVisible(false);
			}
		});

		m_techQueue->onItemSelect([researchableTechs, this, playerFaction](int index) {
			if (index != -1) {
				researchableTechs->deselectItem();
				m_progressBar->setVisible(true);
				m_progressBar->setValue(playerFaction->getTech(m_techQueue->getSelectedItemId().toStdString())->getResearchPercent());
				m_progressBarTech = m_techQueue->getSelectedItemId().toStdString();
			}
			else {
				m_progressBar->setVisible(false);
				m_progressBarTech = "";
				m_description->setText("");
			}
		});

		m_progressBar->onFull([this]() {
			m_techQueue->removeItem(m_techQueue->getSelectedItem());
		});

		tabs->onTabSelect([researchableTechs, playerFaction](const tgui::String& tab) {
			researchableTechs->removeAllItems();
			for (Tech& tech : playerFaction->getAllTechsOfCategory(tab.toStdString())) {
				if (!tech.isResearched() && !tech.isResearching()) {
					researchableTechs->addItem(tech.getName(), tech.getType());
				}
			}
		});
	});
}

void TechGUI::update(Faction* playerFaction) {
	if (playerFaction != nullptr) {
		if (m_progressBar != nullptr && m_progressBarTech != "") {
			m_progressBar->setValue(playerFaction->getTech(m_progressBarTech)->getResearchPercent());
		}
		if (m_techQueue != nullptr) {
			int idx = 0;
			for (auto item : m_techQueue->getItemIds()) {
				const Tech* tech = playerFaction->getTech(item.toStdString());
				if (tech != nullptr) {
					// Remove researched items from tech queue
					if (tech->isResearched()) m_techQueue->removeItemById(item);
					else if (tech->isResearching()) {
						// Set name
						m_techQueue->changeItemById(item, tech->getName() + " (" + std::to_string(idx + 1) + ")");
					}
				}
				else {
					m_techQueue->removeItemById(item);
				}
				idx++;
			}
		}
		if (m_description != nullptr && m_techQueue->getSelectedItemIndex() != -1) {
			const Tech* tech = playerFaction->getTech(m_techQueue->getSelectedItemId().toStdString());
			m_description->setText(tech->getExtendedDescription(playerFaction));
		}
	}
}