#pragma once

#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonActionRegistry.h"


class MainMenu : public OvershellMenu {
    
    void ChooseSplashText(std::filesystem::path directory);
    void DrawMiniMTVOverlay(unsigned char alpha, Vector2 pos);
    void AttractScreen();
    void GotoSongSelect();
    void MainMenuScreen();
public:
    static int logoInt;
    // MainMenu() {};
    // drawing helper functions for other menus

    bool hehe = false;
    bool shouldBreak = false;
    Encore::ButtonActionRegistry buttReg;
    MainMenu() {}
    ~MainMenu() {}
    void Draw();
    void Load();
    void KeyboardInputCallback(SDL_KeyboardEvent* event);
    void ControllerInputCallback(Encore::ControllerEvent event);
    std::string SplashString;
    float TitleAnimTimer = 1;
    int ControllerSelected = 0;
    bool songsLoaded = false;
    bool streamsPaused = false;
    bool stringChosen = false;
    bool albumArtLoaded = false;
    void loadMainMenu();
    inline void loadTitleScreen() {};

    bool songChosen = false;
};


