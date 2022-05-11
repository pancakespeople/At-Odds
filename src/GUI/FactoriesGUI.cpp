#include "gamepch.h"
#include "FactoriesGUI.h"
#include "../Faction.h"
#include "../Sounds.h"

void FactoriesGUI::open(tgui::Gui& gui, Faction& faction) {
	m_icon.open(gui, { "0%", "65%" }, { "2.5%", "5%" }, "data/art/factory.png", "Ship Factories");
	m_icon.setLengthScale(2.0f);

	m_icon.panel->onClick([this, &gui, &faction]() {
		if (m_window != nullptr) {
			m_window->close();
			m_window = nullptr;
			return;
		}
		
		m_window = tgui::ChildWindow::create("Ship Factories");
		m_window->setOrigin(0.5, 0.5);
		m_window->setPosition("50%", "50%");
		m_window->setSize("75%", "75%");
		m_window->getRenderer()->setOpacity(0.75f);

		gui.add(m_window);

		m_window->onClose([this]() {
			m_window = nullptr;
		});

		m_shipFactoryPanel = tgui::Panel::create({ "100%", "75%" });
		m_shipFactoryPanel->setPosition("0%", "25%");
		m_shipFactoryPanel->getRenderer()->setTextureBackground(tgui::Texture());
		m_shipFactoryPanel->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
		m_shipFactoryPanel->getRenderer()->setBorders(1);
		m_shipFactoryPanel->getRenderer()->setBorderColor({125, 125, 125});
		m_shipFactoryGUI.open(gui, faction, m_shipFactoryPanel.get());
		m_window->add(m_shipFactoryPanel);

		auto slider = tgui::Slider::create();
		slider->setPosition("2.5%", "2.5%");
		slider->setSize("95%", "2.5%");
		slider->onValueChange([]() {
			Sounds::playUISound("data/sound/click.wav");
		});
		slider->setStep(1.0f);
		slider->setValue(100.0f);
		m_window->add(slider);
	});
}

void FactoriesGUI::draw(sf::RenderWindow& window) {
	m_icon.draw(window);
}