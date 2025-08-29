//
// Created by maria on 17/12/2024.
//
#include "inputCallbacks.h"

#include "menus/MenuManager.h"
#include "util/enclog.h"
#include "raylib.h"

#include <cstring>

void keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
    // Encore::EncoreLog(LOG_DEBUG, TextFormat("Keyboard key %01i inputted on menu %s",
    // key, ToString(TheMenuManager.currentScreen)) );
    switch (TheMenuManager.currentScreen) { // NOTE: when adding a new Menu
                                            // derivative, you
        // must put its enum value in Screens, and its
        // assignment in this switch/case. You will also
        // add its case to the `ActiveMenu->Draw();`
        // cases.
    case MAIN_MENU:
    case SETTINGS:
    case SETTINGSAUDIOVIDEO:
    case SETTINGSCONTROLLER:
    case SETTINGSGAMEPLAY:
    case SETTINGSKEYBOARD:
    case SETTINGSCREDITS:
    case RESULTS:
    case SONG_SELECT:
    case READY_UP:
    case SOUND_TEST:
    case CACHE_LOADING_SCREEN:
    case CHART_LOADING_SCREEN:
    case GAMEPLAY: {
        TheMenuManager.ActiveMenu->KeyboardInputCallback(key, scancode, action, mods);
        break;
    }
    default:;
    }
}

void gamepadStateCallback(int joypadID, GLFWgamepadstate state) {
    //Encore::EncoreLog(
    //    LOG_DEBUG,
    //    TextFormat(
    //        "Gamepad %01i inputted on menu %s",
    //        joypadID,
    //        ToString(TheMenuManager.currentScreen)
    //    )
    //);
    switch (TheMenuManager.currentScreen) { // NOTE: when adding a new Menu
        // derivative, you
        // must put its enum value in Screens, and its
        // assignment in this switch/case. You will also
        // add its case to the `ActiveMenu->Draw();`
        // cases.
    case MAIN_MENU: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case SETTINGS: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case RESULTS: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case SONG_SELECT: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case READY_UP: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case SOUND_TEST: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case CACHE_LOADING_SCREEN: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case CHART_LOADING_SCREEN: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case GAMEPLAY: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case SETTINGSAUDIOVIDEO: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case SETTINGSCONTROLLER: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case SETTINGSGAMEPLAY: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case SETTINGSKEYBOARD: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    case SETTINGSCREDITS: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    }
}