#include "gamepch.h"

#include <TGUI/TGUI.hpp>

#include "Star.h"
#include "Hyperlane.h"
#include "Camera.h"
#include "Background.h"
#include "Debug.h"
#include "Constellation.h"
#include "Faction.h"
#include "GameState.h"
#include "EffectsEmitter.h"
#include "Sounds.h"
#include "Math.h"
#include "Random.h"
#include "Animation.h"
#include "Pathfinder.h"
#include "GUI.h"
#include "Player.h"
#include "SaveLoader.h"
#include "TextureCache.h"
#include "GUI/DebugConsole.h"
#include "GUI/MainMenu.h"
#include "NameGenerator.h"
#include "MusicPlayer.h"
#include "GUI/OptionsMenu.h"

int main(int argc, const char* argv[])
{
    // Process args
    if (argc > 1) {
        DEBUG_PRINT("Arguments: ");
        for (int i = 1; i < argc; i++) {
            DEBUG_PRINT("\"" << argv[i] << "\"");
            if (std::string(argv[i]) == std::string("--exit")) {
                std::cout << "Exiting game..." << std::endl;
                return 0;
            }
        }
    }
    
    srand(time(NULL));
    
    DEBUG_PRINT("test debug print");

    MainMenu mainMenu;
    NewGameMenu& newGameMenu = mainMenu.getNewGameMenu();
    OptionsMenu& optionsMenu = mainMenu.getOptionsMenu();
    PlayerGUI& playerGui = mainMenu.getNewGameMenu().getPlayerGUI();

    sf::Vector2i resolution = optionsMenu.getResolution();
    OptionsMenu::Settings settings = optionsMenu.getSettings();

    sf::RenderWindow window;
    
    if (settings.fullscreen) {
        window.create(sf::VideoMode(resolution.x, resolution.y), "At Odds", sf::Style::Fullscreen);
    }
    else {
        window.create(sf::VideoMode(resolution.x, resolution.y), "At Odds", sf::Style::Titlebar | sf::Style::Close);
    }

    window.setFramerateLimit(60);

    tgui::GuiSFML gui(window);
    tgui::Theme::setDefault("data/tgui/Black.txt");

    Constellation constellation;
    GameState state(Camera(0, 0, resolution.x, resolution.y));
    Player& player = state.getPlayer();
    UnitGUI& unitGui = playerGui.unitGUI;
    BuildGUI& buildGui = playerGui.buildGUI;
    EffectsEmitter emitter(sf::Vector2i(resolution.x, resolution.y));
    SaveLoader saveLoader;
    DebugConsole console;
    MusicPlayer musicPlayer;

    mainMenu.open(gui, constellation, state);
    
    Background background("data/art/spacebackground1.png", resolution.x, resolution.y);
    newGameMenu.addGameStartCallbacK([&background]() {
        background.setNebulaSeed(Random::randFloat(0.0f, 1.0f));
        DEBUG_PRINT("Randomized nebula seed");
    });

    sf::Shader starShader;
    starShader.loadFromFile("data/shaders/vertexshader.shader", "data/shaders/fragmentshader3.shader");
    starShader.setUniform("resolution", sf::Glsl::Vec2(resolution.x, resolution.y));

    float time = 0;
    int ticks = 0;
    
    Sounds::loadSound("data/sound/woosh1.ogg");
    Sounds::loadSound("data/sound/pew1.wav");

    optionsMenu.updateGameSettings(window, background, gui, emitter, musicPlayer, true);

    sf::Clock fpsClock;
    float fps = 60;
    float updateStep = 1.0f;

    while (window.isOpen() && state.getMetaState() != GameState::MetaState::EXITING &&
        state.getMetaState() != GameState::MetaState::EXIT_AND_SAVE) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            state.getCamera().zoomEvent(event);
            constellation.onEvent(event, window, state);
            state.onEvent(event);
            unitGui.onEvent(event, window, state, constellation.getStars(), playerGui.mainPanel);
            mainMenu.onEvent(event, gui, constellation, state);
            gui.handleEvent(event);
            emitter.onEvent(event);
            playerGui.buildingGUI.onEvent(event, window, gui, state, constellation, playerGui.mainPanel);
            buildGui.onEvent(event, window, state.getLocalViewStar(), constellation.getFaction(state.getPlayer().getFaction()), unitGui, playerGui.mainPanel);
            console.onEvent(event, gui, state);
            playerGui.timescaleGUI.onEvent(event, gui);
            playerGui.planetGUI.onEvent(event, gui, state, constellation.getFaction(state.getPlayer().getFaction()), window, state.getLocalViewStar(), playerGui.mainPanel);
            playerGui.onEvent(event, gui);
        }

        optionsMenu.updateGameSettings(window, background, gui, emitter, musicPlayer);
        if (state.getState() != GameState::State::MAIN_MENU) {
            // Update GUIs
            playerGui.update(window, state, constellation, gui);
        }
        
        // Maintain a constant update rate
        updateStep = playerGui.timescaleGUI.getUpdatesPerSecondTarget() / (1.0f / playerGui.timescaleGUI.getUpdateClock().getElapsedTime().asSeconds());
        for (int i = 0; i < std::round(updateStep); i++) {
            constellation.update(state.getPlayer());
            playerGui.updateSync(window, state, constellation, gui);
        }

        starShader.setUniform("time", time);
        emitter.updateTime(time);

        window.clear();
        
        background.draw(window, emitter);
        
        state.getCamera().update(window, gui.getFocusedLeaf());

        Sounds::updateSounds(state.getPlayer(), state.getCamera());
        musicPlayer.playMusic();

        if (state.getState() == GameState::State::WORLD_VIEW) {
            constellation.draw(window, emitter, starShader, state.getPlayer());
        }
        else if (state.getState() == GameState::State::LOCAL_VIEW) {
            state.getLocalViewStar()->drawLocalView(window, emitter, state.getPlayer(), time);
        }
        
        console.runCommands(constellation, state, window, gui, playerGui);

        buildGui.draw(window, state.getLocalViewStar(), constellation.getFaction(state.getPlayer().getFaction()));
        unitGui.draw(window);
        playerGui.minimapGUI.draw(window, state.getLocalViewStar(), player.getFaction());
        gui.draw();

        window.display();

        ticks++;
        time += fpsClock.getElapsedTime().asSeconds();

        fps = 1.0f / fpsClock.getElapsedTime().asSeconds();
        fpsClock.restart();

        if (state.getMetaState() == GameState::MetaState::LOAD_GAME) {
            saveLoader.loadGame("data/saves/game.save", constellation, state, background);
            mainMenu.close(gui);
            
            state.resetMetaState();
            state.clearCallbacks();
            Sounds::clearSounds();
            gui.removeAllWidgets();

            if (player.getFaction() != -1) {
                playerGui.open(gui, state, constellation, false);
                playerGui.helpWindow.close();
            }
            else {
                playerGui.open(gui, state, constellation, true);
            }

            DEBUG_PRINT("Loaded game");
        }
    }

    if (!window.isOpen()) {
        state.exitGame();
    }

    if (state.getMetaState() == GameState::MetaState::EXIT_AND_SAVE) {
        saveLoader.saveGame("data/saves/game.save", constellation, state, background);
        DEBUG_PRINT("Game saved");
    }

    return 0;
}
