//
// Created by marie on 03/08/2024.
//

#include "overshellRenderer.h"
#include "raylib.h"
#include "raygui.h"
#include "uiUnits.h"
#include "assets.h"
#include "gameMenu.h"
#include "styles.h"

/// std::vector<bool> SlotSelectingState = { false, false, false, false };
/// std::vector<bool> OpenState = { false, false, false, false };
/// std::vector<bool> InstrumentTypeState = { false, false, false, false };

enum OSState {
    OS_ATTRACT,
    OS_PLAYER_SELECTION,
    OS_OPTIONS,
    OS_INSTRUMENT_SELECTIONS
};

int OvershellState[4] { OS_ATTRACT, OS_ATTRACT, OS_ATTRACT, OS_ATTRACT };

void DrawBeacon(int slot, float x, float y, float width, float height, bool top) {
    PlayerManager &playerManager = PlayerManager::getInstance();
    Color overshellBeacon =
        ColorBrightness(playerManager.GetActivePlayer(slot)->AccentColor, -0.75f);
    Color thanksraylib = { overshellBeacon.r, overshellBeacon.g, overshellBeacon.b, 128 };
    float HalfWidth = width / 2;
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
    for (int g = 0; g < 4; g++) {
        DrawRectangleGradientH(x, y, HalfWidth, height, { 0, 0, 0, 0 }, thanksraylib);
        DrawRectangleGradientH(
            x + HalfWidth - 1, y, HalfWidth, height, thanksraylib, { 0, 0, 0, 0 }
        );
    }
    EndBlendMode();
    Color BaseWitAllAlpha = ColorBrightness(GetColor(0x181827FF), -0.25f);
    Color BaseWitNoAlpha = { BaseWitAllAlpha.r, BaseWitAllAlpha.g, BaseWitAllAlpha.b, 0 };
    if (top) {
        DrawRectangleGradientV(x, y, width, height, BaseWitAllAlpha, BaseWitNoAlpha);
    }
}

bool DrawOvershellRectangleHeader(
    float x, float y, float width, float height, std::string username, Color accentColor
) {

    Assets &assets = Assets::getInstance();
    Units &unit = Units::getInstance();
    Rectangle RectPos = { x, y, width, height * 2 };
    bool toReturn = GuiButton({ x, y, width, height }, "");
    DrawRectangleRounded(RectPos, 0.40f, 8, ColorBrightness(accentColor, -0.5f));
    // float Inset = unit.winpct(0.001f);
    // float InsetDouble = Inset * 2;
    // DrawRectangleRounded(
    //     {RectPos.x + Inset, RectPos.y + Inset, RectPos.width - (InsetDouble*1.25f),
    //     RectPos.height - InsetDouble}, 0.40f, 5, ColorBrightness(accentColor, -0.75f)
    //);


    float centerPos = x + (width / 2);
    GameMenu::mhDrawText(
        assets.redHatDisplayBlack,
        username.c_str(),
        { centerPos, (height/4) + y },
        (height/2),
        WHITE,
        assets.sdfShader,
        CENTER
    );
    return toReturn;
}

void OvershellRenderer::DrawTopOvershell(double height) {
    BeginBlendMode(BLEND_ALPHA);
    Units &unit = Units::getInstance();
    PlayerManager &playerManager = PlayerManager::getInstance();
    DrawRectangleGradientV(
        0,
        unit.hpct(height) - 2,
        GetScreenWidth(),
        unit.hinpct(0.025f),
        Color { 0, 0, 0, 128 },
        Color { 0, 0, 0, 0 }
    );
    DrawRectangle(0, 0, (int)GetScreenWidth(), unit.hpct(height), WHITE);
    DrawRectangle(
        0,
        0,
        (int)GetScreenWidth(),
        unit.hpct(height) - unit.hinpct(0.005f),
        ColorBrightness(GetColor(0x181827FF), -0.25f)
    );

    for (int i = 0; i < 4; i++) {
        float OvershellTopLoc = unit.hpct(1.0f) - unit.winpct(0.05f);
        float OvershellLeftLoc =
            (unit.wpct(0.125) + (unit.winpct(0.25) * i)) - unit.winpct(0.1);
        float OvershellCenterLoc = (unit.wpct(0.125) + (unit.winpct(0.25) * i));
        float HalfWidth = OvershellCenterLoc - OvershellLeftLoc;
        if (playerManager.ActivePlayers[i] != -1) {
            DrawBeacon(
                i,
                OvershellLeftLoc,
                0,
                HalfWidth * 2,
                unit.hpct(height) - unit.hinpct(0.005f),
                true
            );
        }
    }
}

/*
 *this is code for the fuckin uh. it the shape of the menu.
if (GuiButton(
                            { OvershellLeftLoc,
                              unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 2)),
                              unit.winpct(0.2f),
                              unit.winpct(0.03f) },
                            playerManager.PlayerList[x].Name.c_str()
                        )) {
    playerManager.AddActivePlayer(x, i);
    CanMouseClick = true;
    SlotSelectingState[i] = false;
    gameMenu.shouldBreak = true;
                        }
*/
bool MenuButton(int slot, int x, std::string string) {
    Units &unit = Units::getInstance();
    float OvershellLeftLoc =
        (unit.wpct(0.125) + (unit.winpct(0.25) * slot)) - unit.winpct(0.1);
    return GuiButton(
        { OvershellLeftLoc,
          unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
          unit.winpct(0.2f),
          unit.winpct(0.03f) },
        string.c_str()
    );
    SETDEFAULTSTYLE();
}
bool OvershellSlider(
    int slot, int x, std::string string, float *value, float step, float min, float max
) {
    Units &unit = Units::getInstance();
    float OvershellLeftLoc =
        (unit.wpct(0.125) + (unit.winpct(0.25) * slot)) - unit.winpct(0.1);
    float height = unit.winpct(0.03f);
    float widthNoHeight = unit.winpct(0.2f) - height;
    Rectangle bounds = { OvershellLeftLoc + height,
                         unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
                         unit.winpct(0.2f) - height - height,
                         height };
    Rectangle confirmBounds = { OvershellLeftLoc + widthNoHeight,
                                unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
                                height,
                                height };
    Assets &assets = Assets::getInstance();

    GuiSlider(bounds, "", "", value, min, max);
    GuiButton(
        { OvershellLeftLoc,
          unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
          height,
          height },
        TextFormat("%1.1f", *value)
    );
    return GuiButton(confirmBounds, "<");
}

bool BNSetting = false;
void OvershellRenderer::DrawBottomOvershell() {
    Assets &assets = Assets::getInstance();
    Units &unit = Units::getInstance();
    GameMenu &gameMenu = TheGameMenu;
    float BottomBottomOvershell = GetScreenHeight() - unit.hpct(0.1f);
    float InnerBottom = BottomBottomOvershell + unit.hinpct(0.005f);
    DrawRectangle(
        0, BottomBottomOvershell, (float)GetScreenWidth(), (float)GetScreenHeight(), WHITE
    );
    DrawRectangle(
        0,
        InnerBottom,
        (float)GetScreenWidth(),
        (float)GetScreenHeight(),
        ColorBrightness(GetColor(0x181827FF), -0.5f)
    );
    int ButtonHeight = unit.winpct(0.03f);
    PlayerManager &playerManager = PlayerManager::getInstance();
    float LeftMin = unit.wpct(0.1);
    float LeftMax = unit.wpct(0.9);
    for (int i = 0; i < 4; i++) {
        bool EmptySlot = true;

        float OvershellTopLoc = unit.hpct(1.0f) - unit.winpct(0.05f);
        float OvershellLeftLoc =
            (unit.wpct(0.125) + (unit.winpct(0.25) * i)) - unit.winpct(0.1);
        float OvershellCenterLoc = (unit.wpct(0.125) + (unit.winpct(0.25) * i));
        float HalfWidth = OvershellCenterLoc - OvershellLeftLoc;
        switch (OvershellState[i]) {
        case OS_PLAYER_SELECTION: {
            // for selecting players
            if (GuiButton(
                    { OvershellLeftLoc,
                      unit.hpct(1.0f) - (unit.winpct(0.03f)),
                      unit.winpct(0.2f),
                      unit.winpct(0.03f) },
                    "Cancel"
                )) {
                CanMouseClick = true;
                OvershellState[i] = OS_ATTRACT;
                gameMenu.shouldBreak = true;
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
            for (int x = 0; x < playerManager.PlayerList.size(); x++) {
                if (playerManager.ActivePlayers[i] == -1) {
                    if (MenuButton(i, x + 1, playerManager.PlayerList[x].Name.c_str())) {
                        playerManager.AddActivePlayer(x, i);
                        CanMouseClick = true;
                        OvershellState[i] = OS_ATTRACT;
                        continue;
                    }
                }
            }

            float InfoLoc = unit.winpct(0.03f) * (playerManager.PlayerList.size() + 1);
            DrawOvershellRectangleHeader(
                                OvershellLeftLoc,
                                OvershellTopLoc - (ButtonHeight * InfoLoc),
                                unit.winpct(0.2f),
                                unit.winpct(0.05f),
                                "Select a player",
                                Color{255,0,255,255}
                            );

            break;
        }
        case OS_OPTIONS: {
            if (DrawOvershellRectangleHeader(
                    OvershellLeftLoc,
                    OvershellTopLoc - (ButtonHeight * 5),
                    unit.winpct(0.2f),
                    unit.winpct(0.05f),
                    playerManager.GetActivePlayer(i)->Name,
                    playerManager.GetActivePlayer(i)->AccentColor
                )) {
                OvershellState[i] = OS_ATTRACT;
                CanMouseClick = true;
                continue;
            }
            if (!BNSetting) {
                if (MenuButton(i, 3, "Breakneck Speed")) {
                    BNSetting = true;
                    continue;
                }
            }
            if (BNSetting) {
                if (OvershellSlider(
                        i,
                        3,
                        "Breakneck Speed",
                        &playerManager.GetActivePlayer(i)->NoteSpeed,
                        1,
                        0.25,
                        3
                    )) {
                    BNSetting = false;
                    continue;
                };
            }
            if (MenuButton(i, 4, "Instrument Type")) {
                OvershellState[i] = OS_INSTRUMENT_SELECTIONS;
                break;
            }
            if (MenuButton(i, 2, "Toggle Bot")) {
                playerManager.GetActivePlayer(i)->Bot =
                    !playerManager.GetActivePlayer(i)->Bot;
            }
            if (MenuButton(i, 1, "Drop Out")) {
                playerManager.RemoveActivePlayer(i);
                OvershellState[i] = OS_ATTRACT;
                CanMouseClick = true;
                continue;
            }
            if (MenuButton(i, 0, "Cancel")) {
                OvershellState[i] = OS_ATTRACT;
                CanMouseClick = true;
            }
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
                    GetScreenHeight(),
                    false
                );

                if (DrawOvershellRectangleHeader(
                        OvershellLeftLoc,
                        OvershellTopLoc,
                        unit.winpct(0.2f),
                        unit.winpct(0.05f),
                        playerManager.GetActivePlayer(i)->Name,
                        playerManager.GetActivePlayer(i)->AccentColor
                    )) {
                    OvershellState[i] = OS_OPTIONS;
                    CanMouseClick = false;
                    continue;
                    // playerManager.RemoveActivePlayer(i);
                    gameMenu.shouldBreak = true;
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
                            LIGHTGRAY
                        )) {
                        CanMouseClick = false;
                        OvershellState[i] = OS_PLAYER_SELECTION;
                        gameMenu.shouldBreak = true;
                        continue;
                    };
                } else {
                    if (DrawOvershellRectangleHeader(
                        OvershellLeftLoc,
                        OvershellTopLoc + unit.winpct(0.01f),
                        unit.winpct(0.2f),
                        unit.winpct(0.04f),
                        "CONNECT CONTROLLER",
                        {0}
                        )) {
                        CanMouseClick = false;
                        OvershellState[i] = OS_PLAYER_SELECTION;
                        gameMenu.shouldBreak = true;
                        continue;
                        };;
                }
            }

            break;
            }
        case OS_INSTRUMENT_SELECTIONS: {
            int ButtonHeight = unit.winpct(0.03f);

            DrawOvershellRectangleHeader(
                OvershellLeftLoc,
                OvershellTopLoc - (ButtonHeight * 5),
                unit.winpct(0.2f),
                unit.winpct(0.05f),
                playerManager.GetActivePlayer(i)->Name,
                playerManager.GetActivePlayer(i)->AccentColor
            );

            if (MenuButton(i, 2, "Classic")) {
                playerManager.GetActivePlayer(i)->ClassicMode = true;
                OvershellState[i] = OS_OPTIONS;
                continue;
            }
            if (MenuButton(i, 1, "Pad")) {
                playerManager.GetActivePlayer(i)->ClassicMode = false;
                OvershellState[i] = OS_OPTIONS;
                continue;
            }
            if (MenuButton(i, 0, "Back")) {
                OvershellState[i] = OS_OPTIONS;
                continue;
            }
            break;
        }
        }
    }
};
