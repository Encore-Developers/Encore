#pragma once
#include "menu.h"

enum Screens {
    MAIN_MENU,
    SONG_SELECT,
    READY_UP,
    GAMEPLAY,
    RESULTS,
    SETTINGS,
    SETTINGSAUDIOVIDEO,
    SETTINGSGAMEPLAY,
    SETTINGSCONTROLLER,
    SETTINGSKEYBOARD,
    SETTINGSCREDITS,
    CALIBRATION,
    CHART_LOADING_SCREEN,
    SOUND_TEST,
    CACHE_LOADING_SCREEN
};

inline const char* ToString(Screens v)
{
    switch (v)
    {
        case MAIN_MENU: return "Main Menu";
        case SONG_SELECT: return "Song Select";
        case READY_UP: return "Ready Up";
        case GAMEPLAY: return "Gameplay";
        case RESULTS: return "Results";
        case SETTINGS: return "Settings";
        case SETTINGSAUDIOVIDEO: return "Audio / Video Settings";
        case SETTINGSGAMEPLAY: return "Gameplay Settings";
        case SETTINGSCONTROLLER: return "Controller Bindings";
        case SETTINGSKEYBOARD: return "Keyboard Bindings";
        case SETTINGSCREDITS: return "Credits";
        case CALIBRATION: return "Calibration";
        case CHART_LOADING_SCREEN: return "Chart Loading Screen";
        case SOUND_TEST: return "Debug Test Menu";
        case CACHE_LOADING_SCREEN: return "Cache Loading Screen";
        default: return "[Unknown Menu]";
    }
}

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
