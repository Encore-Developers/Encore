//
// Created by marie on 03/08/2024.
//

#include "overshellRenderer.h"
#include "raylib.h"
#include "raygui.h"
#include "uiUnits.h"
#include "assets.h"
#include "gameMenu.h"

std::vector<bool> SlotSelectingState = { false, false, false, false };
void DrawBeacon(int slot, float x, float y, float width, float height, bool top) {
    PlayerManager &playerManager = PlayerManager::getInstance();
    Color overshellBeacon =
        ColorBrightness(playerManager.GetActivePlayer(slot)->AccentColor, -0.75f);
    Color thanksraylib = { overshellBeacon.r, overshellBeacon.g, overshellBeacon.b, 128 };
    float HalfWidth = width / 2;
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
    for (int g = 0; g < 4; g++) {
        DrawRectangleGradientH(x, y, HalfWidth, height, { 0,0,0,0}, thanksraylib);
        DrawRectangleGradientH(
            x + HalfWidth - 1, y, HalfWidth, height, thanksraylib, {0,0,0,0}
        );
    }
    EndBlendMode();
    Color BaseWitAllAlpha = ColorBrightness(GetColor(0x181827FF), -0.25f);
    Color BaseWitNoAlpha = {BaseWitAllAlpha.r, BaseWitAllAlpha.g, BaseWitAllAlpha.b, 0};
    if (top) {
        DrawRectangleGradientV(x,y,width,height,BaseWitAllAlpha,BaseWitNoAlpha);
    }
}

void DrawOvershellRectangleHeader(float x, float y, float width, float height, std::string username, Color accentColor) {
    Assets &assets = Assets::getInstance();
    Units &unit = Units::getInstance();
    Rectangle RectPos = {
        x, y, width, height*2
    };
    BeginScissorMode(x, y, width, height);
    DrawRectangleRounded(
        RectPos,
        0.40f,
        5,
        ColorBrightness(accentColor, -0.25f)
    );
    float Inset = unit.winpct(0.001f);
    float InsetDouble = Inset * 2;
    DrawRectangleRounded(
        {RectPos.x + Inset, RectPos.y + Inset, RectPos.width - InsetDouble, RectPos.height - InsetDouble},
        0.40f,
        5,
        ColorBrightness(accentColor, -0.75f)
    );
    EndScissorMode();

    float centerPos = x + (width /2);
    GameMenu::mhDrawText(
        assets.redHatDisplayBlack,
        username.c_str(),
        { centerPos,
          y + unit.winpct(0.01f) },
        unit.winpct(0.03f),
        WHITE,
        assets.sdfShader,
        CENTER
    );
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
            DrawBeacon(i, OvershellLeftLoc, 0, HalfWidth*2, unit.hpct(height) - unit.hinpct(0.005f), true);
        }
    }
}

void OvershellRenderer::DrawBottomOvershell() {
    Assets &assets = Assets::getInstance();
    Units &unit = Units::getInstance();
    GameMenu &gameMenu = TheGameMenu;
    float BottomBottomOvershell = GetScreenHeight() - unit.hpct(0.1f);
    float InnerBottom = BottomBottomOvershell + unit.hinpct(0.005f);
    DrawRectangle(
        0,
        BottomBottomOvershell,
        (float)GetScreenWidth(),
        (float)GetScreenHeight(),
        WHITE
    );
    DrawRectangle(
        0,
        InnerBottom,
        (float)GetScreenWidth(),
        (float)GetScreenHeight(),
        ColorBrightness(GetColor(0x181827FF), -0.5f)
    );

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
        if (SlotSelectingState[i]) {

            if (GuiButton(
                    { OvershellLeftLoc,
                      unit.hpct(1.0f) - (unit.winpct(0.03f)),
                      unit.winpct(0.2f),
                      unit.winpct(0.03f) },
                    "Cancel"
                )) {
                SlotSelectingState[i] = false;
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
                    if (GuiButton(
                            { OvershellLeftLoc,
                              unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 2)),
                              unit.winpct(0.2f),
                              unit.winpct(0.03f) },
                            playerManager.PlayerList[x].Name.c_str()
                        )) {
                        playerManager.AddActivePlayer(x, i);
                        SlotSelectingState[i] = false;
                        gameMenu.shouldBreak = true;
                    }
                }
            }
            float playerNameSize =
                MeasureTextEx(
                    assets.redHatDisplayBlack, "Select a player", unit.winpct(0.03f), 0
                )
                    .x;
            float InfoLoc = unit.winpct(0.03f) * (playerManager.PlayerList.size() + 1);

            DrawRectangle(
                OvershellLeftLoc,
                unit.hpct(1.0f) - InfoLoc - unit.winpct(0.05f),
                unit.winpct(0.2f),
                unit.winpct(0.05f),
                GRAY
            );
            DrawTextEx(
                assets.redHatDisplayBlack,
                "Select a player",
                { OvershellCenterLoc - (playerNameSize / 2),
                  unit.hpct(1.0f) - InfoLoc + unit.winpct(0.01f) - unit.winpct(0.05f) },
                unit.winpct(0.03f),
                0,
                WHITE
            );

        }
        else if (playerManager.ActivePlayers[i] != -1) { // player active
            DrawBeacon(i, OvershellLeftLoc, InnerBottom, HalfWidth*2, GetScreenHeight(), false);
            if (GuiButton(
                    { OvershellLeftLoc,
                      OvershellTopLoc + unit.winpct(0.01f),
                      unit.winpct(0.2f),
                      unit.winpct(0.04f) },
                    ""
                )) {
                playerManager.RemoveActivePlayer(i);
                gameMenu.shouldBreak = true;
            } else {
                DrawOvershellRectangleHeader(
                    OvershellLeftLoc,
                    OvershellTopLoc,
                    unit.winpct(0.2f),
                    unit.winpct(0.05f),
                    playerManager.GetActivePlayer(i)->Name,
                    playerManager.GetActivePlayer(i)->AccentColor
                );

            }
        }
        else { // no active players
            if (GuiButton(
                    { OvershellLeftLoc,
                      OvershellTopLoc + unit.winpct(0.01f),
                      unit.winpct(0.2f),
                      unit.winpct(0.04f) },
                    ""
                )) {
                SlotSelectingState[i] = true;
                gameMenu.shouldBreak = true;
                // playerManager.AddActivePlayer(i, i);
            } else {
                float playerNameSize =
                    MeasureTextEx(assets.redHatDisplayBlack, "JOIN", unit.winpct(0.02f), 0)
                        .x;
                DrawRectangle(
                    OvershellLeftLoc,
                    OvershellTopLoc + unit.winpct(0.01f),
                    unit.winpct(0.2f),
                    unit.winpct(0.04f),
                    DARKGRAY
                );
                DrawTextEx(
                    assets.redHatDisplayBlack,
                    "JOIN",
                    { OvershellCenterLoc - (playerNameSize / 2),
                      OvershellTopLoc + unit.winpct(0.02f) },
                    unit.winpct(0.02f),
                    0,
                    WHITE
                );
            }
        }
    }
};
