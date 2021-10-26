#include "gamepch.h"
#include "ResourceGUI.h"
#include "../Constellation.h"
#include "../Util.h"

void ResourceGUI::open(tgui::Gui& gui) {
	m_resourceGroup = tgui::Group::create();
	m_resourceGroup->setPosition("0%", "0%");
	m_resourceGroup->setSize("33%", "33%");
	m_resourceGroup->setFocusable(false);
	gui.add(m_resourceGroup);

	m_panel = tgui::Panel::create();
	m_panel->setPosition("2.5%", "0%");
	m_panel->setSize("40%", "5%");
	m_panel->getRenderer()->setOpacity(0.75f);
	gui.add(m_panel);

	tgui::ToolTip::setInitialDelay(0);

	auto commonPicture = tgui::Picture::create("data/art/squareicon.png");
	commonPicture->setPosition("0%", "50%");
	commonPicture->setSize("5%", "5%");
	m_panel->add(commonPicture, "commonPicture");

	m_commonLabel = tgui::Label::create();
	m_commonLabel->setPosition("commonPicture.right", "50%");
	m_commonLabel->setOrigin(0.0, 0.5);
	m_commonLabel->setToolTip(tgui::Label::create("Kathium"));
	m_panel->add(m_commonLabel);

	auto uncommonPicture = tgui::Picture::create("data/art/circleicon.png");
	uncommonPicture->setPosition("33%", "50%");
	uncommonPicture->setSize("5%", "5%");
	m_panel->add(uncommonPicture, "uncommonPicture");

	m_uncommonLabel = tgui::Label::create();
	m_uncommonLabel->setPosition("uncommonPicture.right", "50%");
	m_uncommonLabel->setOrigin(0.0, 0.5);
	m_uncommonLabel->setToolTip(tgui::Label::create("Oscillite"));
	m_panel->add(m_uncommonLabel);

	auto rarePicture = tgui::Picture::create("data/art/staricon.png");
	rarePicture->setPosition("66%", "50%");
	rarePicture->setSize("5%", "5%");
	m_panel->add(rarePicture, "rarePicture");

	m_rareLabel = tgui::Label::create();
	m_rareLabel->setPosition("rarePicture.right", "50%");
	m_rareLabel->setOrigin(0.0, 0.5);
	m_rareLabel->setToolTip(tgui::Label::create("Valkrosium"));
	m_panel->add(m_rareLabel);
}

void ResourceGUI::update(Constellation& constellation, Player& player) {
	if (m_resourceGroup != nullptr) {
		if (player.getFaction() != -1) {
			Faction* faction = constellation.getFaction(player.getFaction());
			//auto& resources = faction->getResources();

			if (m_panel != nullptr) {
				if (m_timeUntilChangesUpdate == 0) {
					m_commonChange = faction->getResourceExploitation("COMMON_ORE");
					m_uncommonChange = faction->getResourceExploitation("UNCOMMON_ORE");
					m_rareChange = faction->getResourceExploitation("RARE_ORE");

					m_timeUntilChangesUpdate = 1000;
				}
				else m_timeUntilChangesUpdate--;
				
				m_commonLabel->setText(Util::cutOffDecimal(faction->getResourceCount("COMMON_ORE"), 2) + " (+" + Util::cutOffDecimal(m_commonChange, 2) + ")");
				m_uncommonLabel->setText(Util::cutOffDecimal(faction->getResourceCount("UNCOMMON_ORE"), 2) + " (+" + Util::cutOffDecimal(m_uncommonChange, 2) + ")");
				m_rareLabel->setText(Util::cutOffDecimal(faction->getResourceCount("RARE_ORE"), 2) + " (+" + Util::cutOffDecimal(m_rareChange, 2) + ")");
			}

			/*if (m_resourceGroup->get<tgui::Label>("resourceLabel") == nullptr) {
				auto resourceLabel = tgui::Label::create("Resources");
				resourceLabel->setTextSize(20);
				m_resourceGroup->add(resourceLabel, "resourceLabel");
			}

			int pos = 15;

			for (auto& resource : resources) {
				Resource r;
				r.type = resource.first;

				auto label = m_resourceGroup->get<tgui::Label>(r.getName());

				if (label == nullptr) {
					label = tgui::Label::create();
					label->setPosition("0%", (std::to_string(pos) + "%").c_str());
					label->setTextSize(20);
					m_resourceGroup->add(label, r.getName());
				}

				std::stringstream ss;
				ss << r.getName() << ": " << std::fixed << std::setprecision(1) << resource.second;

				label->setText(ss.str());
				pos += 15;
			}*/
		}
	}
}