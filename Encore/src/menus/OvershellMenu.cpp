#include "OvershellMenu.h"
#include "assets.h"
#include "uiUnits.h"
#include "gameMenu.h"
#include "users/playerManager.h"
#include "OvershellHelper.h"
#include "raygui.h"
#include "raylib.h"

using namespace encOS;
using namespace Encore::RhythmEngine;

bool OvershellKeyboardInputCallback(OvershellMenu *menu, int key, int scancode, int action, int mods) {
    for (int i = 0; i < 4; i++) {
        if (menu->OvershellState[i] == OS_CONTROLLER_ASSIGNMENT && key == KEY_ENTER) {
            ThePlayerManager.GetActivePlayer(i).joypadID = -1;
            menu->OvershellState[i] = OS_OPTIONS;
            return true;
        }
    }
    return false;
}
void DetectControllerType(Player& player) {
    auto type = SDL_GetJoystickType(SDL_GetJoystickFromID(player.joypadID));
    switch (type) {
    case SDL_JOYSTICK_TYPE_GUITAR:
        player.bindingType = GUITAR;
        break;
    case SDL_JOYSTICK_TYPE_DRUM_KIT:
        player.bindingType = DRUMS;
        break;
    default:
        player.bindingType = PAD;
    }
}

OvershellInputState encOS::inputStates[] = {0, 1, 2, 3};
OvershellInputState* OvershellInputState::currentState = nullptr;

bool OvershellControllerInputCallback(OvershellMenu *menu, ControllerEvent event) {
    if (event.channel == InputChannel::WHAMMY || event.channel == InputChannel::INVALID) {
        return false;
    }
    if (event.action == Action::RELEASE) {
        return false;
    }
    bool controllerSignedIn = false;
    for (int i = 0; i < 4; i++) {
        bool thisSlotIsController = false;
        auto playerId = ThePlayerManager.ActivePlayers[i];
        if (menu->OvershellState[i] == OS_CONTROLLER_ASSIGNMENT) {
            auto &player = ThePlayerManager.GetActivePlayer(i);
            player.joypadID = event.slot;
            DetectControllerType(player);
            menu->OvershellState[i] = OS_OPTIONS;
            return true;
        }
        if (playerId != -1) {
            auto &player = ThePlayerManager.GetActivePlayer(i);
            if (player.joypadID == event.slot) {
                controllerSignedIn = true;
                thisSlotIsController = true;
            }
        }
        if (menu->ControllersToAssign[i] == event.slot) {
            controllerSignedIn = true;
            thisSlotIsController = true;
        }
        if (thisSlotIsController) {
            if (event.channel == InputChannel::PAUSE) {
                switch (menu->OvershellState[i]) {
                case OS_ATTRACT:
                    menu->OvershellState[i] = OS_OPTIONS;
                    return true;
                case OS_OPTIONS:
                    menu->OvershellState[i] = OS_ATTRACT;
                    return true;
                }
            }
            if (menu->OvershellState[i] != OS_ATTRACT) {
                inputStates[i].ControllerInput(event);
                return true;
            }
        }

    }
    if ((event.channel == InputChannel::PAUSE || event.channel == InputChannel::LANE_1) && !controllerSignedIn) {
        for (int i = 0; i < 4; i++) {
            if (ThePlayerManager.ActivePlayers[i] == -1 && menu->ControllersToAssign[i] == 0) {
                menu->ControllersToAssign[i] = event.slot;
                menu->OvershellState[i] = OS_PLAYER_SELECTION;
                return true;
            }
        }
    }
    return false;
}

void OvershellMenu::DrawOvershell() {
    Assets &assets = Assets::getInstance();
    Units &unit = Units::getInstance();
    float BottomBottomOvershell = GetRenderHeight() - unit.hpct(0.1f);
    float InnerBottom = BottomBottomOvershell + unit.hinpct(0.005f);
    DrawRectangle(
        0, BottomBottomOvershell, (float)GetRenderWidth(), (float)GetRenderHeight(), WHITE
    );
    DrawRectangle(
        0,
        InnerBottom,
        (float)GetRenderWidth(),
        (float)GetRenderHeight(),
        ColorBrightness(GetColor(0x181827FF), -0.5f)
    );
    GuiSetFont(ASSET(rubik));
    int ButtonHeight = unit.winpct(0.03f);
    PlayerManager &playerManager = ThePlayerManager;
    float LeftMin = unit.wpct(0.1);
    float LeftMax = unit.wpct(0.9);
    for (int i = 0; i < 4; i++) {
        bool EmptySlot = true;
        OvershellInputState& input = inputStates[i];
        input.Begin(this);

        float OvershellTopLoc = unit.hpct(1.0f) - unit.winpct(0.05f);
        float OvershellLeftLoc =
            (unit.wpct(0.125) + (unit.winpct(0.25) * i)) - unit.winpct(0.1);
        float OvershellCenterLoc = (unit.wpct(0.125) + (unit.winpct(0.25) * i));
        float HalfWidth = OvershellCenterLoc - OvershellLeftLoc;
        Color headerUsernameColor;
        if (playerManager.ActivePlayers.at(i) != -1) {
            if (playerManager.GetActivePlayer(i).Bot)
                headerUsernameColor = SKYBLUE;
            else {
                if (playerManager.GetActivePlayer(i).BrutalMode)
                    headerUsernameColor = RED;
                else
                    headerUsernameColor = WHITE;
            }
        }
        switch (OvershellState[i]) {
        case CREATION: {
            static char name[32] = { 0 };
            Rectangle textBoxPosition { OvershellLeftLoc,
                                        unit.hpct(1.0f) - (unit.winpct(0.03f) * 3),
                                        unit.winpct(0.2f),
                                        unit.winpct(0.03f) };
            GuiTextBox(textBoxPosition, name, 32, true);
            input.SetLength(2);
            if (OvershellButton(i, 1, "Confirm")) {
                playerManager.CreatePlayer(name);
                playerManager.AddActivePlayer(playerManager.PlayerList.size() - 1, i);
                CancelButtonActivation = true;
                OvershellState[i] = OS_ATTRACT;
                *name = 0;
                continue;
            }
            if (OvershellButton(i, 0, "Cancel")) {
                OvershellState[i] = OS_ATTRACT;
                *name = 0;
                continue;
            }
            break;
        }
        case OS_PLAYER_SELECTION: {
            // for selecting players
            float InfoLoc = (playerManager.PlayerList.size() + 2);
            if (OvershellButton(i, 0, "Cancel")) {
                CancelButtonActivation = true;
                OvershellState[i] = OS_ATTRACT;
                ControllersToAssign[i] = 0;
            }
            BeginBlendMode(BLEND_MULTIPLIED);
            DrawRectangleRec(
                { OvershellLeftLoc,
                  unit.hpct(1.0f) - (unit.winpct(0.03f)),
                  unit.winpct(0.2f),
                  unit.winpct(0.03f) },
                Color { 255, 0, 0, 128 }
            );
            EndBlendMode();
            int pos = 0;
            if (!playerManager.PlayerList.empty()) {
                for (int x = 0; x < playerManager.PlayerList.size(); x++) {
                    bool playerAlreadyLoggedIn = false;
                    for (int s = 0; s < playerManager.ActivePlayers.size(); s++) {
                        if (playerManager.ActivePlayers[s] == x) {
                            playerAlreadyLoggedIn = true;
                            break;
                        }
                    }
                    if (playerAlreadyLoggedIn) {
                        continue;
                    }
                    if (OvershellButton(
                                i, pos + 2, playerManager.PlayerList[x].Name.c_str()
                            )) {
                        playerManager.AddActivePlayer(x, i);
                        if (ControllersToAssign[i] != 0) {
                            playerManager.GetActivePlayer(i).joypadID = ControllersToAssign[i];
                            DetectControllerType(playerManager.GetActivePlayer(i));
                            ControllersToAssign[i] = 0;
                        }
                        CancelButtonActivation = true;
                        OvershellState[i] = OS_ATTRACT;
                            }
                    pos++;
                }
            }
            input.SetLength(pos + 2);
            DrawOvershellRectangleHeader(
                OvershellLeftLoc,
                OvershellTopLoc - (ButtonHeight * (pos+2)),
                unit.winpct(0.2f),
                unit.winpct(0.05f),
                "Select a player",
                Color { 255, 0, 255, 255 },
                WHITE
            );
            if (OvershellButton(i, 1, "New Profile")) {
                OvershellState[i] = CREATION;
            }
            break;
        }
        case OS_OPTIONS: {

            if (DrawOvershellRectangleHeader(
                    OvershellLeftLoc,
                    OvershellTopLoc - (ButtonHeight * 8),
                    unit.winpct(0.2f),
                    unit.winpct(0.05f),
                    playerManager.GetActivePlayer(i).Name,
                    playerManager.GetActivePlayer(i).AccentColor,
                    headerUsernameColor
                )) {
                OvershellState[i] = OS_ATTRACT;
                CancelButtonActivation = true;
                continue;
            }
            input.SetLength(8);
            if (!BNSetting) {
                if (OvershellButton(
                        i,
                        7,
                        TextFormat(
                            "Breakneck Speed - %4.2fx",
                            playerManager.GetActivePlayer(i).NoteSpeed
                        )
                    )) {
                    BNSetting = true;
                }
            } else {
                if (OvershellSlider(
                    i,
                    7,
                    TextFormat(
                        "Breakneck Speed - %4.2fx",
                        playerManager.GetActivePlayer(i).NoteSpeed
                    ),
                    &playerManager.GetActivePlayer(i).NoteSpeed,
                    0.25,
                    0.25,
                    3
                )) {
                    BNSetting = false;
                };
            }

            const char* typeString;
            switch (playerManager.GetActivePlayer(i).bindingType) {
            case GUITAR:
                typeString = "Guitar";
                break;
            case DRUMS:
                typeString = "Drums";
                break;
            case PAD:
                typeString = "Pad";
                break;
            default:
                typeString = "Unknown";
            }
            if (OvershellButton(i, 6, TextFormat("Instrument Type: %s", typeString))) {
                OvershellState[i] = OS_INSTRUMENT_SELECTIONS;
            }
            auto padId = playerManager.GetActivePlayer(i).joypadID;
            const char* padName = "Unknown";
            if (padId == -2) {
                padName = "All";
            }
            if (padId == -1) {
                padName = "Keyboard";
            }
            if (SDL_GetJoystickFromID(padId)) {
                padName = SDL_GetJoystickNameForID(padId);
            }
            GuiSetStyle(DEFAULT, TEXT_SIZE, (int)unit.hinpct(0.018f));
            if (OvershellButton(i, 5, TextFormat("Controller: %s", padName))) {
                OvershellState[i] = OS_CONTROLLER_ASSIGNMENT;
                break;
            }
            GuiSetStyle(DEFAULT, TEXT_SIZE, (int)unit.hinpct(0.03f));

            playerManager.GetActivePlayer(i).BrutalMode =
                OvershellCheckbox(i, 4, "Brutal Mode", playerManager.GetActivePlayer(i).BrutalMode);
            playerManager.GetActivePlayer(i).Bot =
                OvershellCheckbox(i, 3, "Bot", playerManager.GetActivePlayer(i).Bot);
            playerManager.GetActivePlayer(i).LeftyFlip = OvershellCheckbox(
                i, 2, "Lefty Flip", playerManager.GetActivePlayer(i).LeftyFlip
            );
            if (OvershellButton(i, 1, "Drop Out")) {
                playerManager.SaveSpecificPlayer(i, true);;
                playerManager.RemoveActivePlayer(i);
                OvershellState[i] = OS_ATTRACT;
                CancelButtonActivation = true;
                continue;
            }
            if (OvershellButton(i, 0, "Cancel") || input.backPressed) {
                playerManager.SaveSpecificPlayer(i, true);
                OvershellState[i] = OS_ATTRACT;
                CancelButtonActivation = true;
            }
            break;
        }
        case OS_CONTROLLER_ASSIGNMENT: {
            DrawOvershellRectangleHeader(
               OvershellLeftLoc,
               OvershellTopLoc - (ButtonHeight * 3),
               unit.winpct(0.2f),
               unit.winpct(0.05f),
               playerManager.GetActivePlayer(i).Name,
               playerManager.GetActivePlayer(i).AccentColor,
               headerUsernameColor
            );
            DrawRectangle(OvershellLeftLoc,
                    unit.hpct(1.0f) - (ButtonHeight * 3),
                    unit.winpct(0.2f),
                    (ButtonHeight * 3), ColorBrightness({ 128, 0, 255, 255 }, -0.8));
            OvershellText(i, 2, "Press a button on any controller or press ENTER.");
            break;
        }
        case OS_ATTRACT: {
            input.menuLength = 0;
            if (playerManager.ActivePlayers[i] != -1) {
                // player active
                DrawBeacon(
                    i,
                    OvershellLeftLoc,
                    InnerBottom,
                    HalfWidth * 2,
                    GetRenderHeight(),
                    false,
                    playerManager.GetActivePlayer(i).AccentColor
                );
                Color headerUsernameColor =
                    playerManager.GetActivePlayer(i).Bot ? SKYBLUE : WHITE;
                if (DrawOvershellRectangleHeader(
                        OvershellLeftLoc,
                        OvershellTopLoc,
                        unit.winpct(0.2f),
                        unit.winpct(0.05f),
                        playerManager.GetActivePlayer(i).Name,
                        playerManager.GetActivePlayer(i).AccentColor,
                        headerUsernameColor
                    )) {
                    OvershellState[i] = OS_OPTIONS;
                    CancelButtonActivation = false;
                    continue;
                    // playerManager.RemoveActivePlayer(i);
                };
            } else { // no active players
                // if its the first slot, keyboard can join ALWAYS
                if (IsGamepadAvailable(i) || i == 0) {
                    if (DrawOvershellRectangleHeader(
                            OvershellLeftLoc,
                            OvershellTopLoc + unit.winpct(0.01f),
                            unit.winpct(0.2f),
                            unit.winpct(0.04f),
                            "PRESS START",
                            LIGHTGRAY,
                            RAYWHITE
                        )) {
                        CancelButtonActivation = false;
                        OvershellState[i] = OS_PLAYER_SELECTION;
                        continue;
                    };
                } else {
                    if (DrawOvershellRectangleHeader(
                            OvershellLeftLoc,
                            OvershellTopLoc + unit.winpct(0.01f),
                            unit.winpct(0.2f),
                            unit.winpct(0.04f),
                            "CONNECT CONTROLLER",
                            { 0 },
                            LIGHTGRAY
                        )) {
                        CancelButtonActivation = false;
                        OvershellState[i] = OS_PLAYER_SELECTION;
                        continue;
                    };
                    ;
                }
            }

            break;
        }
        case OS_INSTRUMENT_SELECTIONS: {
            int ButtonHeight = unit.winpct(0.03f);
            Color headerUsernameColor =
                playerManager.GetActivePlayer(i).Bot ? SKYBLUE : WHITE;
            DrawOvershellRectangleHeader(
                OvershellLeftLoc,
                OvershellTopLoc - (ButtonHeight * 4),
                unit.winpct(0.2f),
                unit.winpct(0.05f),
                playerManager.GetActivePlayer(i).Name,
                playerManager.GetActivePlayer(i).AccentColor,
                headerUsernameColor
            );
            input.SetLength(4);

            if (OvershellButton(i, 3, "Guitar")) {
                playerManager.GetActivePlayer(i).bindingType = GUITAR;
                OvershellState[i] = OS_OPTIONS;
            }
            if (OvershellButton(i, 2, "Drums")) {
                playerManager.GetActivePlayer(i).bindingType = DRUMS;
                OvershellState[i] = OS_OPTIONS;
            }
            if (OvershellButton(i, 1, "Pad")) {
                playerManager.GetActivePlayer(i).bindingType = PAD;
                OvershellState[i] = OS_OPTIONS;
            }

            if (OvershellButton(i, 0, "Back")) {
                OvershellState[i] = OS_OPTIONS;
            }
            break;
        }
        }
        input.Reset();
    }
}
