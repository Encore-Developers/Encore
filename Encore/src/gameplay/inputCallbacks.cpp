//
// Created by maria on 17/12/2024.
//
#include "inputCallbacks.h"

#include "menus/MenuManager.h"
#include "util/enclog.h"
#include "raylib.h"
#include "settings/settings.h"
#include "debug/EncoreDebug.h"
#include "rlImGui.h"

#include <cstring>

void keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
    // Encore::EncoreLog(LOG_DEBUG, TextFormat("Keyboard key %01i inputted on menu %s",
    // key, ToString(TheMenuManager.currentScreen)) );

    if (action == GLFW_PRESS) {
        if (key == KEY_F11 || (key == KEY_ENTER && mods & GLFW_MOD_ALT)) {
            TheGameSettings.Fullscreen = !TheGameSettings.Fullscreen;
            TheGameSettings.UpdateFullscreen();
            return;
        }

        if (key == KEY_F3) {
            EncoreDebug::showDebug = !EncoreDebug::showDebug;
        }
    }

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
    rlImGuiPushKeyEvent(key, scancode, action, mods);
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
    default: {
        TheMenuManager.ActiveMenu->ControllerInputCallback(joypadID, state);
        break;
    }
    }
}