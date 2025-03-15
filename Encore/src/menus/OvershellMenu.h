#pragma once

#include "menu.h"
#include "OvershellHelper.h"

enum OSState {
    OS_ATTRACT, // No player/Join
    OS_PLAYER_SELECTION, // Selecting player
    OS_OPTIONS, // Player settings
    OS_INSTRUMENT_SELECTIONS, // Choosing instrument type
                              // (maybe do a submenu like ReadyUpState?)
    CREATION, // Creating a profile
    OS_READY_UP, // Readying up

    // v--- READY UP ---v //
    RU_INST, // Instrument Selection
    RU_DIFF, // Difficulty Selection
    RU_MOD, // Modifier Selection
    RU_FINAL // Overview/Final Ready Up
};

// Persistent Overshell State.
// not piece of shit
// meh. dont feel like its worth it
// i know it kinda limits the overshell not having it, but i think its better to define
// what is "acceptable" in each menu.

/*
class POS {
public:
    int OvershellState[4] { OS_ATTRACT, OS_ATTRACT, OS_ATTRACT, OS_ATTRACT };
    int AvailableControllers = 0;
};

extern POS TheOSState;
*/

class OvershellMenu : public Menu {
public:
    OvershellMenu() {}
    virtual ~OvershellMenu() {}

    int OvershellState[4] { OS_ATTRACT, OS_ATTRACT, OS_ATTRACT, OS_ATTRACT };
    int AvailableControllers = 0;
    virtual void KeyboardInputCallback(int key, int scancode, int action, int mods) = 0;
    virtual void ControllerInputCallback(int joypadID, GLFWgamepadstate state) = 0;
    virtual void Draw() = 0;
    virtual void Load() = 0;
    virtual void DrawOvershell();
    bool CancelButtonActivation;
    bool onNewMenu;
    bool BNSetting = false;
};
