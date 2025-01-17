#pragma once
#include "menu.h"

enum Screens {
    MAIN_MENU,
    SONG_SELECT,
    READY_UP,
    GAMEPLAY,
    RESULTS,
    SETTINGS,
    CALIBRATION,
    CHART_LOADING_SCREEN,
    SOUND_TEST,
    CACHE_LOADING_SCREEN
};

class MenuManager {
public:
    void SwitchScreen(Screens screen);
    void LoadMenu();
    void DrawMenu();
    Screens currentScreen = CACHE_LOADING_SCREEN;
    Menu *ActiveMenu;
    bool onNewMenu = true;
};

extern MenuManager TheMenuManager;
