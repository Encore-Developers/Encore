//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsGameplay.h"

#include "MenuManager.h"
#include "gameMenu.h"
#include "raygui.h"
#include "assets.h"
#include "settings.h"
#include "settingsOptionRenderer.h"
#include "uiUnits.h"
#include "gameplay/enctime.h"
#include "OvershellMenu.h"
#include "util/settings-text.h"

bool ShowGameplaySettings = true;

void SettingsGameplay::Draw() {
    if (!IsWindowReady()) {
        return;
    }

    Units& u = Units::getInstance();
    if (&u == nullptr) {
        return;
    }

    Assets& assets = Assets::getInstance();
    if (&assets == nullptr) {
        return;
    }

    SettingsOld& settingsMain = SettingsOld::getInstance();
    if (&settingsMain == nullptr) {
        return;
    }

    SongTime& enctime = TheSongTime;
    if (&enctime == nullptr) {
    }

    settingsOptionRenderer sor;
    const float boxWidthPct = 0.55f;

    if (TheSongList.curSong != nullptr && IsTextureReady(TheSongList.curSong->albumArtBlur)) {
        GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    } else {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
    }
    DrawRectangle(u.LeftSide, 0, u.winpct(1.0f), GetScreenHeight(), Color{0, 0, 0});

    float SidebarLeft = u.LeftSide + u.winpct(0.70f);
    float SidebarWidth = u.wpct(0.235f);
    float SidebarTop = u.hinpct(0.15f);
    float SidebarHeight = u.hpct(0.85f);
    float SidebarHeaderHeight = u.hinpct(0.10f);
    float borderWidth = u.winpct(0.002f);
    float innerTop = SidebarTop + borderWidth;

    DrawLineEx({SidebarLeft - borderWidth, SidebarTop}, {SidebarLeft - borderWidth, SidebarTop + SidebarHeight}, borderWidth, WHITE);
    DrawLineEx({SidebarLeft + SidebarWidth, SidebarTop}, {SidebarLeft + SidebarWidth, SidebarTop + SidebarHeight}, borderWidth, WHITE);
    DrawLineEx({SidebarLeft - borderWidth, SidebarTop + SidebarHeight}, {SidebarLeft + SidebarWidth + borderWidth, SidebarTop + SidebarHeight}, borderWidth, WHITE);
    DrawRectangle(SidebarLeft, SidebarTop, SidebarWidth, SidebarHeight, Color{31, 31, 50, 255});

    struct SidebarContent {
        const char* header;
        const char* body;
    };
    SidebarContent sidebarContents[] = {
        // sidebar text
        // fullscreen
        {
            "Fullscreen",
            "TBD"
        },
        // scan Songs
        {
            "Scan Songs",
            "TBD"
        }
    };

    static int selectedIndex = 0;
    Vector2 mousePos = GetMousePosition();
    bool isHovering = false;

    const char* headerText = sidebarContents[selectedIndex].header;
    const char* sidebarBodyText = sidebarContents[selectedIndex].body;
    float headerFontSize = u.hinpct(0.030f);
    float headerLineSpacing = headerFontSize * 1.2f;
    std::vector<std::string> headerLines = split(headerText, "\n");
    float maxHeaderWidth = 0;
    for (const std::string& line : headerLines) {
        if (IsFontReady(assets.rubikBold)) {
            Vector2 lineSize = MeasureTextEx(assets.rubikBold, line.c_str(), headerFontSize, 0);
            if (lineSize.x > maxHeaderWidth) {
                maxHeaderWidth = lineSize.x;
            }
        }
    }
    float currentHeaderY = innerTop + u.hinpct(0.02f);
    for (const std::string& line : headerLines) {
        float lineX = SidebarLeft + (SidebarWidth - maxHeaderWidth) / 2;
        if (IsFontReady(assets.rubikBold)) {
            DrawTextEx(assets.rubikBold, line.c_str(), {lineX, currentHeaderY}, headerFontSize, 0, WHITE);
        }
        currentHeaderY += headerLineSpacing;
    }
    float bodyFontSize = u.hinpct(0.030f);
    float lineSpacing = bodyFontSize * 1.2f;
    std::vector<std::string> lines = split(sidebarBodyText, "\n");
    float currentY = SidebarTop + SidebarHeaderHeight + u.hinpct(0.02f);
    for (const std::string& line : lines) {
        if (IsFontReady(assets.rubik)) {
            Vector2 lineSize = MeasureTextEx(assets.rubik, line.c_str(), bodyFontSize, 0);
            float lineX = SidebarLeft + (SidebarWidth - lineSize.x) / 2;
            DrawTextEx(assets.rubik, line.c_str(), {lineX, currentY}, bodyFontSize, 0, WHITE);
            currentY += lineSpacing;
        }
    }

    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();
    DrawOvershell();

    float TextPlacementTB = u.hpct(0.05f);
    float TextPlacementLR = u.wpct(0.05f);
    if (IsFontReady(assets.rubik)) {
        DrawTextEx(assets.rubik, "Settings", {TextPlacementLR, u.hpct(0.027f)}, u.hinpct(0.042f), 0, LIGHTGRAY);
    }
    if (IsFontReady(assets.redHatDisplayBlack) && IsShaderReady(assets.sdfShader)) {
        GameMenu::mhDrawText(assets.redHatDisplayBlack, "GAMEPLAY", {TextPlacementLR, TextPlacementTB}, u.hinpct(0.125f), WHITE, assets.sdfShader, LEFT);
    }

    float settingsOffsetX = 0.0f;
    float settingsOffsetY = 0.0f;
    float EntryFontSize = u.hinpct(0.03f);
    float EntryHeight = u.hinpct(0.06f) + 30.0f - 50.0f + 10.0f + 7.0f;
    float EntryTop = TextPlacementTB + u.hinpct(0.125f) + u.hinpct(0.01f) + settingsOffsetY - 30.0f - 2.0f - 2.0f;
    float verticalGap = 0.0f;
    float boxLeft = u.LeftSide + u.winpct(0.025f) + settingsOffsetX + 75.0f - 50.0f - 2.0f;
    float boxWidth = u.wpct(boxWidthPct) + 50.0f + 17.0f + 7.0f - 2.0f - 2.0f;
    float OptionLeft = boxLeft;
    float OptionWidth = boxWidth;
    float boxPadding = 0.0f;
    Color boxBackground = Color{31, 31, 50, 255};
    Color boxBorder = WHITE;
    Color glowColor = Color{142, 13, 148, 220};
    float highlightBorderWidth = 4.0f;

    float scanButtonWidth = OptionWidth;
    float scanButtonHeight = EntryHeight + 10.0f;
    float toggleButtonWidth = ((OptionWidth / 2) * 0.3f) - 30.0f;
    float toggleOffset = 50.0f;

    Color activeColor = Color{255, 105, 180, 255};
    int defaultColor = GuiGetStyle(BUTTON, BASE_COLOR_PRESSED);

    int settingOffset = 0;
    float fullscreenTop = EntryTop + (EntryHeight + verticalGap) * settingOffset;
    Rectangle fullscreenBoxRect = {boxLeft - borderWidth, fullscreenTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
    DrawRectangle(boxLeft - borderWidth, fullscreenTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
    DrawRectangle(boxLeft, fullscreenTop, boxWidth, EntryHeight, boxBackground);
    Vector2 fullscreenTextSize = IsFontReady(assets.rubikBold) ? MeasureTextEx(assets.rubikBold, "Fullscreen", EntryFontSize, 0) : Vector2{100, 20};
    if (IsFontReady(assets.rubikBold)) {
        DrawTextEx(assets.rubikBold, "Fullscreen", {boxLeft + u.winpct(0.01f), fullscreenTop + (EntryHeight - fullscreenTextSize.y) / 2}, EntryFontSize, 0, WHITE);
    }
    Rectangle offButtonRect = {OptionLeft + OptionWidth - 2 * toggleButtonWidth - toggleOffset, fullscreenTop, toggleButtonWidth, EntryHeight};
    Rectangle onButtonRect = {OptionLeft + OptionWidth - toggleButtonWidth - toggleOffset, fullscreenTop, toggleButtonWidth, EntryHeight};
    if (CheckCollisionPointRec(mousePos, offButtonRect) || CheckCollisionPointRec(mousePos, onButtonRect)) {
        selectedIndex = 0;
        isHovering = true;
        DrawRectangleLinesEx(fullscreenBoxRect, highlightBorderWidth, glowColor);
    }
    // might redo later - Jaydenz
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, settingsMain.fullscreen ? defaultColor : ColorToInt(activeColor));
    if (GuiButton(offButtonRect, "Off")) {
        if (settingsMain.fullscreen) {
            settingsMain.fullscreen = false;
            if (IsWindowFullscreen()) {
                ToggleFullscreen();
            }
            settingsMain.saveSettings(settingsMain.getDirectory() / "settings.json");
        }
    }
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, settingsMain.fullscreen ? ColorToInt(activeColor) : defaultColor);
    if (GuiButton(onButtonRect, "On")) {
        if (!settingsMain.fullscreen) {
            settingsMain.fullscreen = true;
            if (!IsWindowFullscreen()) {
                ToggleFullscreen();
            }
            settingsMain.saveSettings(settingsMain.getDirectory() / "settings.json");
        }
    }
    if (!settingsMain.fullscreen) {
        DrawRectangleLinesEx(offButtonRect, highlightBorderWidth, glowColor);
    } else {
        DrawRectangleLinesEx(onButtonRect, highlightBorderWidth, glowColor);
    }
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, defaultColor);

    settingOffset++;
    float scanSongsTop = EntryTop + (EntryHeight + verticalGap) * settingOffset;
    Rectangle scanSongsBoxRect = {boxLeft - borderWidth, scanSongsTop - borderWidth, boxWidth + 2 * borderWidth, scanButtonHeight + 2 * borderWidth};
    DrawRectangle(boxLeft - borderWidth, scanSongsTop - borderWidth, boxWidth + 2 * borderWidth, scanButtonHeight + 2 * borderWidth, boxBorder);
    DrawRectangle(boxLeft, scanSongsTop, boxWidth, scanButtonHeight, boxBackground);
    Vector2 scanSongsTextSize = IsFontReady(assets.rubikBold) ? MeasureTextEx(assets.rubikBold, "Scan Songs", EntryFontSize, 0) : Vector2{100, 20};
    if (IsFontReady(assets.rubikBold)) {
        DrawTextEx(assets.rubikBold, "Scan Songs", {boxLeft + u.winpct(0.01f), scanSongsTop + (scanButtonHeight - scanSongsTextSize.y) / 2}, EntryFontSize, 0, WHITE);
    }
    Rectangle scanButtonRect = {OptionLeft + OptionWidth - scanButtonWidth, scanSongsTop, scanButtonWidth, scanButtonHeight};
    if (CheckCollisionPointRec(mousePos, scanButtonRect)) {
        selectedIndex = 1;
        isHovering = true;
        DrawRectangleLinesEx(scanSongsBoxRect, highlightBorderWidth, glowColor);
    }
    if (GuiButton(scanButtonRect, "Scan Songs")) {
        if (TheGameSettings.SongPaths.empty()) {
            TraceLog(LOG_ERROR, "SongPaths is empty. Cannot scan songs.");
        } else {
            try {
                TraceLog(LOG_INFO, "Starting song scan with %d paths", TheGameSettings.SongPaths.size());
                for (const auto& path : TheGameSettings.SongPaths) {
                    TraceLog(LOG_INFO, "Scanning path: %s", path.c_str());
                }
                TheSongList.ScanSongs(TheGameSettings.SongPaths);
                TraceLog(LOG_INFO, "Song scan completed successfully");
            } catch (const std::exception& e) {
                TraceLog(LOG_ERROR, "Error during song scan: %s", e.what());
            } catch (...) {
                TraceLog(LOG_ERROR, "Unknown error during song scan");
            }
        }
    }

    if (!isHovering) {
        selectedIndex = 0;
    }
}

#include <raylib.h>

void SettingsGameplay::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
}

void SettingsGameplay::ControllerInputCallback(int joypadID, GLFWgamepadstate state) {
    if (state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
}

void SettingsGameplay::Load() {
    SettingsOld& settingsMain = SettingsOld::getInstance();
    // Ensure window state matches settings
    if (settingsMain.fullscreen && !IsWindowFullscreen()) {
        ToggleFullscreen();
    } else if (!settingsMain.fullscreen && IsWindowFullscreen()) {
        ToggleFullscreen();
    }
}

void SettingsGameplay::Save() {
    SettingsOld& settingsMain = SettingsOld::getInstance();
    settingsMain.saveSettings(settingsMain.getDirectory() / "settings.json");
}