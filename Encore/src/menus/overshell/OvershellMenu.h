#pragma once

#include "../menu.h"
#include "users/playerManager.h"

enum OSState {
    OS_ATTRACT, // No player/Join
    OS_PLAYER_SELECTION, // Selecting player
    OS_OPTIONS, // Player settings
    OS_INSTRUMENT_SELECTIONS, // Choosing instrument type
                              // (maybe do a submenu like ReadyUpState?)
    OS_COLOR_PROFILE_SELECTION,
    OS_COLOR_PROFILE_TYPE_SELECTOR,
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
//
// class BaseOSOption {
// public:
//     virtual ~BaseOSOption() = default;
//     // display text
//     std::string Name;
//     virtual void draw(int pos, int slot);
//     std::function<void()> action;
//     /* this would be a per-class thing, example for buttons
//     if (OvershellButton(Name, pos, slot)) {
//         action();
//     }
//      */
// };
//
// class OSOption : public BaseOSOption {
//     void draw(int pos, int slot) override {
//
//     };
// };

class OvershellMenu : public Menu {
public:
    static const std::unordered_map<std::string, ControllerBindingType> hardcodedControllerTypes;

    OvershellMenu() {}
    virtual ~OvershellMenu() {}

    SDL_JoystickID ControllersToAssign[MAX_PLAYERS] = {0};
    Encore::ProfileManager::ColorProfileType ColorProfileType[MAX_PLAYERS] { Encore::ProfileManager::PLASTIC };
    int OvershellState[MAX_PLAYERS] { 0 };
    int AvailableControllers = 0;
    bool isOSOpen();
    bool dropInDropOut = true;
    virtual void KeyboardInputCallback(SDL_KeyboardEvent* event) = 0;
    virtual void ControllerInputCallback(Encore::ControllerEvent event) = 0;
    virtual void Draw() = 0;
    virtual void Load() = 0;
    virtual void DrawOvershell();
    bool CancelButtonActivation;
    bool onNewMenu;
    bool BNSetting = false;
    bool hasOvershell = true;
};

bool OvershellKeyboardInputCallback(OvershellMenu *menu, SDL_KeyboardEvent* event);
bool OvershellControllerInputCallback(OvershellMenu *menu, Encore::ControllerEvent event);