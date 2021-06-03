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

int main(int argc, const char* argv[])
{
    srand(time(NULL));
    
    DEBUG_PRINT("test debug print");

    MainMenu mainMenu;
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
    
    mainMenu.open(gui, constellation, state);
    
    Background background("data/art/spacebackground1.png", resolution.x, resolution.y);

    sf::Shader starShader;
    starShader.loadFromFile("data/shaders/vertexshader.shader", "data/shaders/fragmentshader3.shader");
    starShader.setUniform("resolution", sf::Glsl::Vec2(resolution.x, resolution.y));

    float time = 0;
    int ticks = 0;
    
    Sounds::loadSound("data/sound/woosh1.ogg");
    Sounds::loadSound("data/sound/pew1.wav");

    optionsMenu.updateGameSettings(window, background, gui, emitter, true);

    sf::Clock fpsClock;
    float fps = 60;

    sf::Clock updateClock;
    float updatesPerSecondTarget = 60.0f;
    float updateStep = 1.0f;

    // Process args
    if (argc > 1) {
        DEBUG_PRINT("Arguments: ");
        for (int i = 1; i < argc; i++) {
            DEBUG_PRINT("\"" << argv[i] << "\"");
            if (std::string(argv[i]) == std::string("--exit")) {
                DEBUG_PRINT("Exiting game...");
                state.exitGame();
            }
        }
    }

    while (window.isOpen() && state.getMetaState() != GameState::MetaState::EXITING &&
        state.getMetaState() != GameState::MetaState::EXIT_AND_SAVE) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            state.getCamera().zoomEvent(event);
            constellation.onEvent(event, window, state);
            state.onEvent(event);
            unitGui.onEvent(event, window, state, constellation.getStars());
            mainMenu.onEvent(event, gui, constellation, state);
            gui.handleEvent(event);
            emitter.onEvent(event);
            buildGui.onEvent(event, window, state.getLocalViewStar(), state.getPlayer());
            console.onEvent(event, gui, state);
        }

        optionsMenu.updateGameSettings(window, background, gui, emitter);
        if (state.getState() != GameState::State::MAIN_MENU) {
            unitGui.update(window, state.getLocalViewStar(), state.getPlayer().getFaction());
        }
        
        // Maintain a constant update rate
        updateStep = updatesPerSecondTarget / (1.0f / updateClock.getElapsedTime().asSeconds());
        for (int i = 0; i < std::round(updateStep); i++) {
            constellation.update();
            updateClock.restart();
        }
        
        starShader.setUniform("time", time);

        window.clear();
        
        background.draw(window);
        
        state.getCamera().update(window);

        if (state.getState() == GameState::State::WORLD_VIEW) {
            constellation.draw(window, starShader);
        }
        else if (state.getState() == GameState::State::LOCAL_VIEW) {
            state.getLocalViewStar()->drawLocalView(window, emitter, state.getPlayer(), time);
        }
        
        console.runCommands(constellation, state, window);

        buildGui.draw(window, state.getLocalViewStar(), player);
        unitGui.draw(window);
        gui.draw();

        window.display();

        ticks++;
        time += fpsClock.getElapsedTime().asSeconds();

        fps = 1.0f / fpsClock.getElapsedTime().asSeconds();
        fpsClock.restart();

        if (state.getMetaState() == GameState::MetaState::LOAD_GAME) {
            saveLoader.loadGame("data/saves/game.save", constellation, state);
            mainMenu.close();
            state.resetMetaState();

            if (player.getFaction() != -1) {
                playerGui.open(gui);
                playerGui.helpWindow.close();
            }
            else {
                playerGui.unitGUI.open(gui);
            }

            DEBUG_PRINT("Loaded game");
        }
    }

    if (!window.isOpen()) {
        state.exitGame();
    }

    if (state.getMetaState() == GameState::MetaState::EXIT_AND_SAVE) {
        saveLoader.saveGame("data/saves/game.save", constellation, state);
        DEBUG_PRINT("Game saved");
    }

    return 0;
}