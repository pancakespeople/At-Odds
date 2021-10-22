#include "gamepch.h"
#include "TechGUI.h"
#include "../Faction.h"

void TechGUI::open(tgui::Gui& gui, Faction* playerFaction) {
	m_icon.open(gui, tgui::Layout2d("0%", "70%"), tgui::Layout2d("2.5%", "5%"), "data/art/techicon.png");

	m_icon.panel->onClick([&gui, playerFaction]() {
		auto window = tgui::ChildWindow::create("Tech");
		window->setOrigin(0.5, 0.5);
		window->setPosition("50%", "50%");
		window->setSize("75%", "75%");
		window->getRenderer()->setOpacity(0.75f);
		gui.add(window);

		auto researchableTechs = tgui::ListBox::create();
		researchableTechs->setPosition("2.5%", "2.5%");
		researchableTechs->setSize("50% - 2.5%", "50% - 2.5%");
		window->add(researchableTechs, "researchableTechs");

		auto researchLabel = tgui::Label::create("Available Research");
		researchLabel->setOrigin(0.5, 0.0);
		researchLabel->setPosition("researchableTechs.left + researchableTechs.width / 2", "0%");
		window->add(researchLabel);

		auto techQueue = tgui::ListBox::create();
		techQueue->setPosition("50%", "2.5%");
		techQueue->setSize("50% - 2.5%", "50% - 2.5%");
		window->add(techQueue, "techQueue");

		auto queueLabel = tgui::Label::create("Queue");
		queueLabel->setOrigin(0.5, 0.0);
		queueLabel->setPosition("techQueue.left + techQueue.width / 2", "0%");
		window->add(queueLabel);

		auto descriptionLabel = tgui::Label::create();
		descriptionLabel->setPosition("2.5%", "52.5%");
		window->add(descriptionLabel);

		int idx = 0;
		for (const Tech& tech : playerFaction->getAvailableTechs()) {
			researchableTechs->addItem(tech.getName());
			researchableTechs->setItemData(idx, tech.getType());
			idx++;
		}

		researchableTechs->onItemSelect([researchableTechs, descriptionLabel](int index) {
			if (index != -1) {
				Tech tech = Tech(researchableTechs->getItemData<std::string>(index));
				descriptionLabel->setText(tech.getDescription());
			}
			else descriptionLabel->setText("");
		});
	});
}