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
#include "Fonts.h"
#include "GUI/DebugInfo.h"
#include "Renderer.h"
#include "Script.h"
#include "Keybindings.h"

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

    //window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    tgui::GuiSFML gui(window);
    gui.setFont(Fonts::MAIN_FONT_PATH);
    tgui::Theme::setDefault("data/tgui/newUI.txt");

    Renderer renderer(resolution);
    Background& background = renderer.background;
    Constellation constellation;
    GameState state(Camera(0, 0, resolution.x, resolution.y));
    Player& player = state.getPlayer();
    UnitGUI& unitGui = playerGui.unitGUI;
    BuildGUI& buildGui = playerGui.buildGUI;
    SaveLoader saveLoader;
    DebugConsole console;
    MusicPlayer musicPlayer;
    DebugInfo debugInfo(window);

    mainMenu.spawnArena(gui, constellation, state, playerGui);
    mainMenu.open(gui, constellation, state);
    newGameMenu.addGameStartCallbacK([&background]() {
        background.setNebulaSeed(Random::randFloat(0.0f, 1.0f));
        DEBUG_PRINT("Randomized nebula seed");
    });

    Keybindings::addDefaultKeybindings();

    float time = 0.0f;
    float gameTime = 0.0f;
    int ticks = 0;
    int updates = 0;
    int updatesPerFrame = 0;
    const int maxUpdateIterations = 100;
    
    optionsMenu.updateGameSettings(window, renderer, gui, musicPlayer, state.getCamera(), true);

    sf::Clock fpsClock;
    float fps = 60.0f;
    float frameTime = 0.0f;
    float updateStep = 0.0f;

    newGameMenu.addGameStartCallbacK([&background, &time, &musicPlayer]() {
        background.setNebulaSeed(Random::randFloat(0.0f, 1.0f));
        DEBUG_PRINT("Randomized nebula seed");
        musicPlayer.startMusic(time);
    });

    while (window.isOpen() && state.getMetaState() != GameState::MetaState::EXITING &&
        state.getMetaState() != GameState::MetaState::EXIT_AND_SAVE) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            gui.handleEvent(event);
            state.getCamera().onEvent(event, gui.getFocusedLeaf());
            constellation.onEvent(event, window, renderer, state);
            state.onEvent(event);
            buildGui.onEvent(event, window, renderer, state.getLocalViewStar(), constellation.getFaction(state.getPlayer().getFaction()), unitGui, playerGui.mainPanel);
            unitGui.onEvent(event, window, renderer, state, constellation, playerGui.mainPanel, playerGui.minimapGUI);
            mainMenu.onEvent(event, gui, constellation, state);
            playerGui.buildingGUI.onEvent(event, window, renderer, gui, state, constellation, playerGui.mainPanel);
            console.onEvent(event, gui, state);
            playerGui.timescaleGUI.onEvent(event, gui, state);
            playerGui.planetGUI.onEvent(event, gui, state, constellation.getFaction(state.getPlayer().getFaction()), window, renderer, state.getLocalViewStar(), playerGui.mainPanel, constellation);
            playerGui.onEvent(event, gui);
            renderer.onEvent(event, window);
        }

        optionsMenu.updateGameSettings(window, renderer, gui, musicPlayer, state.getCamera());
        if (state.getState() != GameState::State::MAIN_MENU) {
            // Update GUIs
            playerGui.update(window, renderer, state, constellation, gui);
        }
        
        // Maintain a constant update rate
        if (state.getTimescale() != 0) {
            updateStep += frameTime;
        }

        int iterations = 0;
        while (updateStep >= 1.0f / state.getUpdatesPerSecondTarget()) {
            state.restartUpdateClock();
            
            constellation.update(state.getPlayer(), renderer.effects);
            playerGui.updateSync(window, state, constellation, gui);
            mainMenu.updateArena(updates, constellation, state);
            player.update(state);
            
            updates++;
            updatesPerFrame++;
            iterations++;
            updateStep -= 1.0f / state.getUpdatesPerSecondTarget();
            gameTime += 1.0f / state.getUpdatesPerSecondTarget();

            if (iterations >= maxUpdateIterations) {
                updateStep = 0.0f;
                break;
            }
        }

        renderer.effects.updateTime(time, gameTime);

        window.clear();
        renderer.clear();

        background.draw(renderer, state.getCamera());

        state.getCamera().update(renderer);

        Sounds::updateSounds(state.getPlayer(), state.getCamera(), constellation.getAlliances());
        musicPlayer.update(time);

        if (state.getState() == GameState::State::WORLD_VIEW) {
            constellation.draw(window, renderer, state.getPlayer());
        }
        else if (state.getState() == GameState::State::LOCAL_VIEW) {
            state.getLocalViewStar()->drawLocalView(window, renderer, state.getPlayer(), constellation.getAlliances(), time);
        }
        
        console.runCommands(constellation, state, window, renderer, gui, playerGui);
        
        //mainMenu.drawPreview(window, emitter, state, time);
        renderer.displayToWindow(window, state);
        //debugInfo.draw(window);
        playerGui.draw(window, renderer, state, constellation, player);
        gui.draw();

        window.display();

        //debugInfo.update(fps, fpsClock.getElapsedTime().asMilliseconds(), ticks, updatesPerFrame);
        
        ticks++;
        time += fpsClock.getElapsedTime().asSeconds();

        fps = 1.0f / fpsClock.getElapsedTime().asSeconds();
        frameTime = fpsClock.getElapsedTime().asSeconds();
        updatesPerFrame = 0;
        fpsClock.restart();

        if (state.getMetaState() == GameState::MetaState::LOAD_GAME) {
            if (saveLoader.loadGame("data/saves/game.save", constellation, state, background)) {
                mainMenu.close(gui);
                mainMenu.setForceOpen(false);
                
                state.setArenaGame(false);
                state.resetMetaState();
                state.clearCallbacks();
                Sounds::clearSounds();
                gui.removeAllWidgets();

                musicPlayer.startMusic(time);

                if (player.getFaction() != -1) {
                    playerGui.open(gui, state, constellation, PlayerGUIState::PLAYER);
                    playerGui.helpWindow.close();
                }
                else {
                    playerGui.open(gui, state, constellation, PlayerGUIState::SPECTATOR);
                }

                DEBUG_PRINT("Loaded game");
            }
            else {
                state.resetMetaState();

                DEBUG_PRINT("Failed to load game");
            }
        }
    }

    if (!window.isOpen()) {
        state.exitGame();
    }

    if (state.getMetaState() == GameState::MetaState::EXIT_AND_SAVE && !mainMenu.isForceOpen()) {
        saveLoader.saveGame("data/saves/game.save", constellation, state, background);
        DEBUG_PRINT("Game saved");
    }

    return 0;
}
