//
// Created by marie on 02/05/2024.
//


#include "scenes.h"




void Scenes::SwitchScreen(Screens screen) {
    Scenes::currentScreen = screen;
    switch (screen) {
        case MENU:
            // reset lerps
            lerpCtrl.removeLerp("MENU_LOGO");
            break;

        case SONG_SELECT:
            break;
        case SONG_LOADING_SCREEN:
            break;
        case INSTRUMENT_SELECT:
            break;
        case DIFFICULTY_SELECT:
            break;
        case GAMEPLAY:
            break;
        case RESULTS:
            break;
        case SETTINGS:
            break;
    }
}