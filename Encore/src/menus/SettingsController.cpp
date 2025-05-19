//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsController.h"
#include "MenuManager.h"
#include "gameMenu.h"
#include "assets.h"
#include "raygui.h"
#include "uiUnits.h"
#include "util/settings-text.h"


void SettingsController::Draw() {
    TraceLog(LOG_INFO, "SettingsController::Draw entered, options.size=%zu, selectedIndex=%d",
             options.size(), selectedIndex);
    if (!TheSongList.curSong) {
        TraceLog(LOG_ERROR, "TheSongList.curSong is null");
        Units& u = Units::getInstance();
        Assets& assets = Assets::getInstance();
        DrawRectangle(50, 50, GetScreenWidth() - 100, GetScreenHeight() - 100, Fade(BLACK, 0.8f));
        DrawTextEx(assets.rubikBold, "Error: No song selected", {100, 100}, u.hinpct(0.04f), 0, RED);
        return;
    }

    Units& u = Units::getInstance();
    Assets& assets = Assets::getInstance();
    TraceLog(LOG_INFO, "Units=%p, Assets initialized, rubikBold=%p", &u, &assets.rubikBold);
    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    DrawRectangle(u.LeftSide, 0, u.winpct(1.0f), GetScreenHeight(), Color{0, 0, 0});
    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();

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

    const char* headerText = sidebarContents[selectedIndex].header;
    const char* sidebarBodyText = sidebarContents[selectedIndex].body;
    TraceLog(LOG_INFO, "Sidebar header: %s, body: %s", headerText, sidebarBodyText);
    float headerFontSize = u.hinpct(0.030f);
    float headerLineSpacing = headerFontSize * 1.2f;
    std::vector<std::string> headerLines = split(headerText, "\n");
    float maxHeaderWidth = 0;
    for (const std::string& line : headerLines) {
        Vector2 lineSize = MeasureTextEx(assets.rubikBold, line.c_str(), headerFontSize, 0);
        if (lineSize.x > maxHeaderWidth) maxHeaderWidth = lineSize.x;
    }
    float currentHeaderY = innerTop + u.hinpct(0.02f);
    for (const std::string& line : headerLines) {
        float lineX = SidebarLeft + (SidebarWidth - maxHeaderWidth) / 2;
        DrawTextEx(assets.rubikBold, line.c_str(), {lineX, currentHeaderY}, headerFontSize, 0, WHITE);
        currentHeaderY += headerLineSpacing;
    }
    float bodyFontSize = u.hinpct(0.030f);
    float lineSpacing = bodyFontSize * 1.2f;
    std::vector<std::string> lines = split(sidebarBodyText, "\n");
    float currentY = SidebarTop + SidebarHeaderHeight + u.hinpct(0.02f);
    for (const std::string& line : lines) {
        Vector2 lineSize = MeasureTextEx(assets.rubik, line.c_str(), bodyFontSize, 0);
        float lineX = SidebarLeft + (SidebarWidth - lineSize.x) / 2;
        DrawTextEx(assets.rubik, line.c_str(), {lineX, currentY}, bodyFontSize, 0, WHITE);
        currentY += lineSpacing;
    }

    float TextPlacementTB = u.hpct(0.05f);
    float TextPlacementLR = u.wpct(0.05f);
    DrawTextEx(assets.rubik, "Settings", {TextPlacementLR, u.hpct(0.027f)}, u.hinpct(0.042f), 0, LIGHTGRAY);
    GameMenu::mhDrawText(assets.redHatDisplayBlack, "CONTROLLER BINDINGS", {TextPlacementLR, TextPlacementTB}, u.hinpct(0.125f), WHITE, assets.sdfShader, LEFT);

    float settingsOffsetX = 0.0f;
    float settingsOffsetY = 0.0f;
    float EntryFontSize = u.hinpct(0.03f);
    float EntryHeight = u.hinpct(0.06f) + 30.0f - 50.0f + 10.0f + 7.0f;
    float EntryTop = TextPlacementTB + u.hinpct(0.15f);
    float verticalGap = 0.0f;
    float boxLeft = u.LeftSide + u.winpct(0.025f) + settingsOffsetX + 75.0f - 50.0f - 2.0f;
    float boxWidth = u.wpct(boxWidthPct) + 50.0f + 17.0f + 7.0f - 2.0f - 2.0f;
    float OptionLeft = boxLeft;
    float OptionWidth = boxWidth;
    Color boxBackground = Color{31, 31, 50, 255};
    Color boxBorder = WHITE;
    Color glowColor = Color{142, 13, 148, 220};
    float highlightBorderWidth = 4.0f;

    Vector2 mousePos = GetMousePosition();
    isHovering = false;

    TraceLog(LOG_INFO, "Rendering %zu controller entries", options.size());
    for (size_t i = 0; i < options.size(); ++i) {
        float optionTop = EntryTop + (EntryHeight + verticalGap) * i;
        Rectangle optionBoxRect = {boxLeft - borderWidth, optionTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
        DrawRectangle(boxLeft - borderWidth, optionTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
        DrawRectangle(boxLeft, optionTop, boxWidth, EntryHeight, boxBackground);

        std::string label = options[i].first;
        Vector2 labelSize = MeasureTextEx(assets.rubikBold, label.c_str(), EntryFontSize, 0);
        DrawTextEx(assets.rubikBold, label.c_str(), {boxLeft + u.winpct(0.01f), optionTop + (EntryHeight - labelSize.y) / 2}, EntryFontSize, 0, WHITE);

        float buttonWidth = u.winpct(0.15f);
        Rectangle buttonRect = {OptionLeft + OptionWidth - buttonWidth, optionTop, buttonWidth, EntryHeight};
        std::string buttonText = keybinds.getControllerStr(GLFW_JOYSTICK_1, *options[i].second, settings.controllerType, 0);
        TraceLog(LOG_INFO, "Rendering controller bind %s: %s (code=%d)", label.c_str(), buttonText.c_str(), *options[i].second);
        if (static_cast<int>(i) == bindingOption) buttonText = "Press button...";
        if (CheckCollisionPointRec(mousePos, buttonRect)) {
            selectedIndex = i + 1;
            isHovering = true;
            DrawRectangleLinesEx(optionBoxRect, highlightBorderWidth, glowColor);
        }
        if (GuiButton(buttonRect, buttonText.c_str())) {
            bindingOption = i;
        }
        if (static_cast<int>(i) == bindingOption) {
            DrawRectangleLinesEx(buttonRect, highlightBorderWidth, glowColor);
        }
    }

    if (!isHovering) {
        selectedIndex = 0;
    }

    GameMenu::DrawBottomOvershell();
    DrawOvershell();
}

void SettingsController::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;

    if (bindingOption >= 0) {
        bindingOption = -1;
        return;
    }

    if (key == GLFW_KEY_DOWN) {
        selectedIndex = (selectedIndex + 1) % sidebarContents.size();
        if (selectedIndex == 0) selectedIndex = 1;
    } else if (key == GLFW_KEY_UP) {
        selectedIndex = (selectedIndex - 1 + sidebarContents.size()) % sidebarContents.size();
        if (selectedIndex == 0) selectedIndex = sidebarContents.size() - 1;
    } else if (key == GLFW_KEY_ENTER) {
        if (selectedIndex > 0) bindingOption = selectedIndex - 1;
    } else if (key == GLFW_KEY_ESCAPE) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
}

void SettingsController::ControllerInputCallback(int joypadID, GLFWgamepadstate state) {
    static GLFWgamepadstate prevState;
    if (bindingOption >= 0) {
        for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST; ++i) {
            if (state.buttons[i] && !prevState.buttons[i]) {
                *options[bindingOption].second = i;
                Save();
                TraceLog(LOG_INFO, "Bound %s to button %d (%s)",
                         options[bindingOption].first.c_str(), i,
                         keybinds.getControllerStr(joypadID, i, settings.controllerType, 0).c_str());
                bindingOption = -1;
                prevState = state;
                return;
            }
        }
        for (int i = 0; i < GLFW_GAMEPAD_AXIS_LAST; ++i) {
            float value = state.axes[i];
            float prevValue = prevState.axes[i];
            if (fabs(value) > 0.5f && fabs(prevValue) <= 0.5f) {
                *options[bindingOption].second = -1 - i;
                Save();
                TraceLog(LOG_INFO, "Bound %s to axis %d direction %s",
                         options[bindingOption].first.c_str(), i,
                         (value > 0 ? "+" : "-"));
                bindingOption = -1;
                prevState = state;
                return;
            }
        }
    }
    if (state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
    prevState = state;
}

void SettingsController::Load() {
    TraceLog(LOG_INFO, "SettingsController: Loaded controller binds from settings-old.json");
    TraceLog(LOG_INFO, "Controller Type: %d", settings.controllerType);
    TraceLog(LOG_INFO, "4K Controller: [%d, %d, %d, %d]",
             settings.controller4K[0], settings.controller4K[1],
             settings.controller4K[2], settings.controller4K[3]);
    TraceLog(LOG_INFO, "Overdrive: %d, Pause: %d",
             settings.controllerOverdrive, settings.controllerPause);
}

void SettingsController::Save() {
    settings.saveOldSettings(settings.getDirectory() / "settings-old.json");
    settings.syncKeybindsToGame();
    TraceLog(LOG_INFO, "SettingsController: Saved controller binds to settings-old.json");
}
