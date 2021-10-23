#include "gamepch.h"
#include "TechGUI.h"
#include "../Faction.h"

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

		auto researchableTechs = tgui::ListBox::create();
		researchableTechs->setPosition("2.5%", "2.5%");
		researchableTechs->setSize("50% - 2.5%", "50% - 2.5%");
		m_window->add(researchableTechs, "researchableTechs");

		auto researchLabel = tgui::Label::create("Available Research");
		researchLabel->setOrigin(0.5, 0.0);
		researchLabel->setPosition("researchableTechs.left + researchableTechs.width / 2", "0%");
		m_window->add(researchLabel);

		auto techQueue = tgui::ListBox::create();
		techQueue->setPosition("50%", "2.5%");
		techQueue->setSize("50% - 2.5%", "50% - 2.5%");
		m_window->add(techQueue, "techQueue");

		auto queueLabel = tgui::Label::create("Queue");
		queueLabel->setOrigin(0.5, 0.0);
		queueLabel->setPosition("techQueue.left + techQueue.width / 2", "0%");
		m_window->add(queueLabel);

		auto researchButton = tgui::Button::create("Research");
		researchButton->setPosition("2.5%", "52.5%");
		researchButton->setVisible(false);
		m_window->add(researchButton);

		auto descriptionLabel = tgui::Label::create();
		descriptionLabel->setPosition("2.5%", "62.5%");
		m_window->add(descriptionLabel);

		for (const Tech& tech : playerFaction->getTechs()) {
			if (!tech.isResearched() && !tech.isResearching()) {
				researchableTechs->addItem(tech.getName(), tech.getType());
			}
			else if (tech.isResearching()) {
				techQueue->addItem(tech.getName(), tech.getType());
			}
		}

		researchableTechs->onItemSelect([researchableTechs, techQueue, descriptionLabel, researchButton, playerFaction](int index) {
			if (index != -1) {
				Tech tech = Tech(researchableTechs->getSelectedItemId().toStdString());
				std::string techType = tech.getType();
				descriptionLabel->setText(tech.getDescription());
				
				researchButton->setVisible(true);
				researchButton->onClick.disconnectAll();
				researchButton->onClick([tech, researchableTechs, techQueue, playerFaction]() {
					playerFaction->setResearchingTech(tech.getType(), true);
					researchableTechs->removeItem(researchableTechs->getSelectedItem());
					techQueue->addItem(tech.getName());
				});
			}
			else {
				descriptionLabel->setText("");
				researchButton->setVisible(false);
			}
		});
	});
}