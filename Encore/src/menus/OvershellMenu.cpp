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
bool OvershellControllerInputCallback(OvershellMenu *menu, ControllerEvent event) {
    if (event.channel == InputChannel::WHAMMY || event.channel == InputChannel::INVALID) {
        return false;
    }
    for (int i = 0; i < 4; i++) {
        if (menu->OvershellState[i] == OS_CONTROLLER_ASSIGNMENT) {
            ThePlayerManager.GetActivePlayer(i).joypadID = event.slot;
            menu->OvershellState[i] = OS_OPTIONS;
            return true;
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
    int ButtonHeight = unit.winpct(0.03f);
    PlayerManager &playerManager = ThePlayerManager;
    float LeftMin = unit.wpct(0.1);
    float LeftMax = unit.wpct(0.9);
    for (int i = 0; i < 4; i++) {
        bool EmptySlot = true;

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
            if (OvershellButton(i, 1, "Confirm")) {
                playerManager.CreatePlayer(name);
                playerManager.AddActivePlayer(playerManager.PlayerList.size() - 1, i);
                CancelButtonActivation = true;
                OvershellState[i] = OS_ATTRACT;
                continue;
            }
            if (OvershellButton(i, 0, "Cancel")) {
                OvershellState[i] = CREATION;
                continue;
            }
            break;
        }
        case OS_PLAYER_SELECTION: {
            // for selecting players
            float InfoLoc = (playerManager.PlayerList.size() + 2);
            DrawOvershellRectangleHeader(
                OvershellLeftLoc,
                OvershellTopLoc - (ButtonHeight * InfoLoc),
                unit.winpct(0.2f),
                unit.winpct(0.05f),
                "Select a player",
                Color { 255, 0, 255, 255 },
                WHITE
            );
            if (GuiButton(
                    { OvershellLeftLoc,
                      unit.hpct(1.0f) - (unit.winpct(0.03f)),
                      unit.winpct(0.2f),
                      unit.winpct(0.03f) },
                    "Cancel"
                )) {
                CancelButtonActivation = true;
                OvershellState[i] = OS_ATTRACT;
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
            if (!playerManager.PlayerList.empty()) {
                for (int x = 0; x < playerManager.PlayerList.size(); x++) {
                    if (playerManager.ActivePlayers[i] == -1) {
                        if (OvershellButton(
                                i, x + 1, playerManager.PlayerList[x].Name.c_str()
                            )) {
                            playerManager.AddActivePlayer(x, i);
                            CancelButtonActivation = true;
                            OvershellState[i] = OS_ATTRACT;
                        }
                    }
                }
            }
            if (OvershellButton(i, playerManager.PlayerList.size() + 1, "New Profile")) {
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
                    continue;
                }
            }
            if (BNSetting) {
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
                    continue;
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
                break;
            }
            if (OvershellButton(i, 5, "Assign Controller")) {
                OvershellState[i] = OS_CONTROLLER_ASSIGNMENT;
                break;
            }
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
            if (OvershellButton(i, 0, "Cancel")) {
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
                            "JOIN",
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
                continue;
            }
            break;
        }
        }
    }
}
