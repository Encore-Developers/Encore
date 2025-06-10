// Created by Jaydenz on 04/29/2025.
//

#include "SettingsKeyboard.h"
#include "MenuManager.h"
#include "gameMenu.h"
#include "assets.h"
#include "uiUnits.h"
#include "util/settings-text.h"
#include "OvershellMenu.h"
#include "raygui.h"

void SettingsKeyboard::Draw() {
    Units& u = Units::getInstance();
    Assets& assets = Assets::getInstance();
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
    GameMenu::mhDrawText(assets.redHatDisplayBlack, "KEYBOARD BINDINGS", {TextPlacementLR, TextPlacementTB}, u.hinpct(0.125f), WHITE, assets.sdfShader, LEFT);

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
    Color boxBackground = Color{31, 31, 50, 255};
    Color boxBorder = WHITE;
    Color glowColor = Color{142, 13, 148, 220};
    float highlightBorderWidth = 4.0f;

    Vector2 mousePos = GetMousePosition();
    isHovering = false;

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
        std::string buttonText = (*options[i].second == -2) ? "Unbound" : keybinds.getKeyStr(*options[i].second);
        if (static_cast<int>(i) == bindingOption) buttonText = "Press key...";
        if (CheckCollisionPointRec(mousePos, buttonRect)) {
            selectedIndex = i + 1; // Offset for sidebarContents
            isHovering = true;
            DrawRectangleLinesEx(optionBoxRect, highlightBorderWidth, glowColor);
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                auto [bindType, bindIndex] = getBindTypeAndIndex(i);
                settings.rebindKey(bindType, bindIndex);
                TraceLog(LOG_INFO, "Unbound %s via right-click", label.c_str());
            }
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

std::pair<std::string, int> SettingsKeyboard::getBindTypeAndIndex(size_t optionIndex) {
    switch (optionIndex) {
        case 0: return {"keybinds4K", 0};
        case 1: return {"keybinds4K", 1};
        case 2: return {"keybinds4K", 2};
        case 3: return {"keybinds4K", 3};
        case 4: return {"keybinds5K", 0};
        case 5: return {"keybinds5K", 1};
        case 6: return {"keybinds5K", 2};
        case 7: return {"keybinds5K", 3};
        case 8: return {"keybinds5K", 4};
        case 9: return {"keybindOverdrive", -1};
        case 10: return {"keybindOverdriveAlt", -1};
        case 11: return {"keybindPause", -1};
        default: return {"", -1};
    }
}

void SettingsKeyboard::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;

    if (bindingOption >= 0) {
        if (key != GLFW_KEY_ESCAPE) { // Allow any key except ESC
            *options[bindingOption].second = key;
            auto [bindType, bindIndex] = getBindTypeAndIndex(bindingOption);
            settings.rebindKey(bindType, bindIndex);
            Save();
            TraceLog(LOG_INFO, "Bound %s to key %d (%s)", options[bindingOption].first.c_str(), key, keybinds.getKeyStr(key).c_str());
        }
        bindingOption = -1;
        return;
    }

    if (key == GLFW_KEY_DOWN) {
        selectedIndex = (selectedIndex + 1) % sidebarContents.size();
        if (selectedIndex == 0) selectedIndex = 1; // fixed header
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

void SettingsKeyboard::ControllerInputCallback(int joypadID, GLFWgamepadstate state) {
    if (state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
}

void SettingsKeyboard::Load() {
    TraceLog(LOG_INFO, "SettingsKeyboard: Loaded keybinds from settings-old.json");
}

void SettingsKeyboard::Save() {
    settings.saveOldSettings(settings.getDirectory() / "settings-old.json");
    settings.syncKeybindsToGame();
    TraceLog(LOG_INFO, "SettingsKeyboard: Saved keybinds to settings-old.json");
}