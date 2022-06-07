#include "gamepch.h"
#include "NewGameMenu.h"
#include "../Constellation.h"
#include "../Sounds.h"
#include "MainMenu.h"
#include "../Random.h"

void NewGameMenu::open(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu) {
	auto guiWindow = GUI::ChildWindow::create("New Game");
	guiWindow->setSize({ "50%", "50%" });
	guiWindow->setPosition("50%", "50%");
	guiWindow->setOrigin({ 0.5f, 0.5f });
	guiWindow->getRenderer()->setOpacity(0.75f);
	guiWindow->setTitleButtons(GUI::ChildWindow::TitleButton::None);
	gui.add(guiWindow);
	m_window = guiWindow;

	auto mainMenuButton = GUI::Button::create("<- Main Menu");
	mainMenuButton->setPosition("5%", "90%");
	mainMenuButton->onPress([this, &gui, &constellation, &state, mainMenu]() {
		close();
		mainMenu->open(gui, constellation, state);
		});
	guiWindow->add(mainMenuButton);

	auto startGameButton = GUI::Button::create("Start Game ->");
	startGameButton->setPosition("95% - width", "90%");
	startGameButton->onPress(&NewGameMenu::startNewGame, this, std::ref(gui), std::ref(constellation), std::ref(state), mainMenu);
	guiWindow->add(startGameButton);

	// Stars slider

	auto numStarsLabel = tgui::Label::create("Number of stars");
	numStarsLabel->setPosition("5%", "0%");
	guiWindow->add(numStarsLabel);

	auto numStarsSlider = tgui::Slider::create(20.0f, 500.0f);
	numStarsSlider->setValue(50.0f);
	numStarsSlider->setStep(1.0f);
	numStarsSlider->setPosition("5%", "5%");
	numStarsSlider->onValueChange(&NewGameMenu::onStarsSliderChange, this, std::ref(gui));
	guiWindow->add(numStarsSlider, "starSlider");

	auto numStarsNumLabel = tgui::Label::create(std::to_string(static_cast<int>(numStarsSlider->getValue())));
	numStarsNumLabel->setPosition("starSlider.left + starSlider.width", "starSlider.top");
	guiWindow->add(numStarsNumLabel, "numStars");

	//
	// Factions slider

	auto numFactionsLabel = tgui::Label::create("Number of factions");
	numFactionsLabel->setPosition("5%", "15%");
	guiWindow->add(numFactionsLabel);

	auto numFactionsSlider = tgui::Slider::create(2.0f, 15.0f);
	numFactionsSlider->setValue(4.0f);
	numFactionsSlider->setStep(1.0f);
	numFactionsSlider->setPosition("5%", "20%");
	numFactionsSlider->onValueChange(&NewGameMenu::onFactionsSliderChange, this, std::ref(gui));
	guiWindow->add(numFactionsSlider, "factionSlider");

	auto numFactionsNumLabel = tgui::Label::create(std::to_string(static_cast<int>(numFactionsSlider->getValue())));
	numFactionsNumLabel->setPosition("factionSlider.left + factionSlider.width", "factionSlider.top");
	guiWindow->add(numFactionsNumLabel, "numFactions");

	//
	
	auto seedLabel = tgui::Label::create("Seed");
	seedLabel->setPosition("5%", "30%");
	guiWindow->add(seedLabel);

	auto seedEditBox = tgui::EditBox::create();
	seedEditBox->setPosition("5%", "35%");
	seedEditBox->setSize("20%", "5%");
	seedEditBox->setText(Random::randString(10));
	guiWindow->add(seedEditBox, "seedEditBox");

	auto spectateCheckbox = tgui::CheckBox::create("Spectate");
	spectateCheckbox->setPosition("5%", "45%");
	guiWindow->add(spectateCheckbox, "spectateCheckBox");

	tgui::ToolTip::setInitialDelay(0);

	auto alliancesCheckbox = tgui::CheckBox::create("Alliances");
	alliancesCheckbox->setPosition("5%", "55%");
	alliancesCheckbox->setToolTip(tgui::Label::create("Divides all factions into 2 alliances"));
	guiWindow->add(alliancesCheckbox, "alliancesCheckbox");

	auto wiamCheckbox = tgui::CheckBox::create("The World Is Against Me");
	wiamCheckbox->setPosition("5%", "65%");
	wiamCheckbox->setToolTip(tgui::Label::create("Puts all AI factions into an alliance against the player"));
	guiWindow->add(wiamCheckbox, "wiamCheckbox");
	
	alliancesCheckbox->onCheck([wiamCheckbox]() {
		wiamCheckbox->setChecked(false);
	});
	
	wiamCheckbox->onCheck([alliancesCheckbox]() {
		alliancesCheckbox->setChecked(false);
	});
}

void NewGameMenu::close() {
	if (m_window != nullptr)
		m_window->close();
}

void NewGameMenu::onStarsSliderChange(tgui::Gui& gui) {
	auto slider = gui.get<tgui::Slider>("starSlider");
	auto label = gui.get<tgui::Label>("numStars");

	label->setText(std::to_string(static_cast<int>(slider->getValue())));
}

void NewGameMenu::onFactionsSliderChange(tgui::Gui& gui) {
	auto slider = gui.get<tgui::Slider>("factionSlider");
	auto label = gui.get<tgui::Label>("numFactions");

	label->setText(std::to_string(static_cast<int>(slider->getValue())));
}

void NewGameMenu::startNewGame(tgui::Gui& gui, Constellation& constellation, GameState& state, MainMenu* mainMenu) {
	int starsNum = m_window->get<tgui::Slider>("starSlider")->getValue();
	int factionsNum = m_window->get<tgui::Slider>("factionSlider")->getValue();

	Camera camera = state.getCamera();

	state = GameState(camera);
	constellation = Constellation();

	auto seedEditBox = m_window->get<tgui::EditBox>("seedEditBox");
	Random::setGeneratorSeed(std::hash<std::string>{}(seedEditBox->getText().toStdString()));

	//constellation.generateModernMegaRobustFinalConstellation(800, 800, starsNum);
	constellation.generateTheReallyFinalRobustConstellationIMeanItReally(3000, 3000, starsNum);
	//constellation.setupStars();

	auto& stars = constellation.getStars();

	constellation.generateFactions(factionsNum);
	constellation.generateNeutralSquatters();
	constellation.generatePirates();

	auto alliancesCheckbox = m_window->get<tgui::CheckBox>("alliancesCheckbox");
	if (alliancesCheckbox->isChecked()) {
		constellation.setupAlliances();
	}

	auto wiamCheckbox = m_window->get<tgui::CheckBox>("wiamCheckbox");
	if (wiamCheckbox->isChecked()) {
		constellation.setupWorldIsAgainstMe();
	}

	state.getCamera().setPos(stars[0]->getPos());
	state.getCamera().resetZoom();

	bool spectate = m_window->get<tgui::CheckBox>("spectateCheckBox")->isChecked();

	gui.removeAllWidgets();
	state.clearCallbacks();
	Sounds::clearSounds();

	std::vector<Faction>& factions = constellation.getFactions();

	if (!spectate) {
		factions[0].controlByPlayer(state.getPlayer());

		for (int i = 1; i < factions.size(); i++) {
			factions[i].setAIEnabled(true);
		}

		state.changeToLocalView(factions[0].getCapital());
		state.getCamera().setPos(factions[0].getCapital()->getLocalViewCenter());

		m_playerGui.open(gui, state, constellation, PlayerGUIState::PLAYER);
	}
	else {
		for (int i = 0; i < factions.size(); i++) {
			factions[i].setAIEnabled(true);
		}

		state.changeToWorldView();

		m_playerGui.open(gui, state, constellation, PlayerGUIState::SPECTATOR);
	}

	m_playerGui.setVisible(gui, true);
	constellation.onStart();

	for (auto& func : m_gameStartCallbacks) {
		func();
	}

	mainMenu->setForceOpen(false);
	state.setArenaGame(false);
	close();
}