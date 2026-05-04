#pragma once

#include "menu.h"

enum OSState {
    OS_ATTRACT, // No player/Join
    OS_PLAYER_SELECTION, // Selecting player
    OS_OPTIONS, // Player settings
    OS_INSTRUMENT_SELECTIONS, // Choosing instrument type
                              // (maybe do a submenu like ReadyUpState?)
    OS_CONTROLLER_ASSIGNMENT,
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

    SDL_JoystickID ControllersToAssign[4] = {0, 0, 0, 0};
    int OvershellState[4] { OS_ATTRACT, OS_ATTRACT, OS_ATTRACT, OS_ATTRACT };
    int AvailableControllers = 0;
    bool dropInDropOut = true;
    virtual void KeyboardInputCallback(int key, int scancode, int action, int mods) = 0;
    virtual void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) = 0;
    virtual void Draw() = 0;
    virtual void Load() = 0;
    virtual void DrawOvershell();
    bool CancelButtonActivation;
    bool onNewMenu;
    bool BNSetting = false;
    bool hasOvershell = true;
};

bool OvershellKeyboardInputCallback(OvershellMenu *menu, int key, int scancode, int action, int mods);
bool OvershellControllerInputCallback(OvershellMenu *menu, Encore::RhythmEngine::ControllerEvent event);