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

int main()
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

    tgui::Gui gui(window);
    tgui::Theme blackTheme("data/tgui/Black.txt");
    tgui::Theme::setDefault(&blackTheme);

    Constellation constellation;
    GameState state(Camera(0, 0, resolution.x, resolution.y));
    UnitGUI unitGUI;
    EffectsEmitter emitter(sf::Vector2i(resolution.x, resolution.y));
    
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

    while (window.isOpen() && state.getState() != GameState::State::EXITING) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            state.getCamera().zoomEvent(event);
            constellation.onEvent(event, window, state);
            state.onEvent(event);
            unitGUI.onEvent(event, window, state, constellation.getStars());
            mainMenu.onEvent(event, gui, constellation, state);
            gui.handleEvent(event);
            emitter.onEvent(event);
        }

        optionsMenu.updateGameSettings(window, background, gui, emitter);
        if (state.getState() != GameState::State::MAIN_MENU) {
            unitGUI.update(window, state.getLocalViewStar(), state.getPlayer().getFaction());
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
        unitGUI.draw(window);
        
        state.getCamera().update(window);

        if (state.getState() == GameState::State::WORLD_VIEW) {
            constellation.draw(window, starShader);
        }
        else if (state.getState() == GameState::State::LOCAL_VIEW) {
            state.getLocalViewStar()->drawLocalView(window, emitter, state.getPlayer(), time);
        }
        
        gui.draw();

        window.display();

        ticks++;
        time += fpsClock.getElapsedTime().asSeconds();

        fps = 1.0f / fpsClock.getElapsedTime().asSeconds();
        fpsClock.restart();
    }

    return 0;
}