#include "gamepch.h"
#include "NewsGUI.h"
#include "../Faction.h"

void NewsGUI::open(tgui::Gui& gui) {
	tgui::Panel::Ptr topPanel = tgui::Panel::create();
	topPanel->setPosition("85%", "0%");
	topPanel->setSize("25%", "2.5%");
	topPanel->setOrigin(0.5, 0.0);
	gui.add(topPanel);

	tgui::Label::Ptr newsLabel = tgui::Label::create("News");
	newsLabel->setOrigin(0.5f, 0.5f);
	newsLabel->setPosition("50%", "50%");
	topPanel->add(newsLabel);

	m_box = tgui::ChatBox::create();
	m_box->setPosition("70%", "2.5%");
	m_box->setSize("30%", "20%");
	gui.add(m_box);
}

void NewsGUI::addEvent(const std::string& text) {
	if (m_box != nullptr) {
		m_box->addLine(text, tgui::Color::White);
	}
}

void NewsGUI::update(Faction* playerFaction) {
	if (playerFaction != nullptr) {
		std::deque<std::string>& news = playerFaction->getNewsEvents();
		while (news.size() > 0) {
			addEvent(news.front());
			news.pop_front();
		}
	}
}
