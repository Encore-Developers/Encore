// Created by Jaydenz on 04/29/2025.
//

#include "SettingsKeyboard.h"

#include "SettingsMenu.h"
#include "../MenuManager.h"
#include "../main/MainMenu.h"
#include "assets.h"
#include "../uiUnits.h"
#include "util/settings-text.h"
#include "../overshell/OvershellMenu.h"
#include "raygui.h"
#include "../overshell/OvershellHelper.h"
#include "menus/locale/Locale.h"

void SettingsKeyboard::Draw() {
    Units& u = Units::getInstance();
    Assets& assets = Assets::getInstance();
    GameMenu::DrawAlbumArtBackground();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 128 });
    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();

    float TextPlacementTB = u.hpct(0.05f);
    float TextPlacementLR = u.wpct(0.05f);
    Encore::Text::lDrawText(assets.rubik, "settings.header.keyboard", {u.LeftSide, u.hpct(0.027f)}, u.hinpct(0.042f), LIGHTGRAY, LEFT);
    Encore::Text::lDrawText(assets.redHatDisplayBlack, "settings.header.main", {u.LeftSide, TextPlacementTB}, u.hinpct(0.125f), WHITE, LEFT);

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
    /*
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
*/
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

void SettingsKeyboard::KeyboardInputCallback(SDL_KeyboardEvent* event) {

}

void SettingsKeyboard::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {
    if (event.action == Encore::RhythmEngine::Action::PRESS) {
        switch (event.channel) {
        case Encore::RhythmEngine::InputChannel::LANE_2: {
            Save();
            TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
            break;
        }
        }
    }
}

void SettingsKeyboard::Load() {
    TraceLog(LOG_INFO, "SettingsKeyboard: Loaded keybinds from settings-old.json");
}

void SettingsKeyboard::Save() {
   //  settings.saveOldSettings(settings.getDirectory() / "settings-old.json");
    // settings.syncKeybindsToGame();
    TraceLog(LOG_INFO, "SettingsKeyboard: Saved keybinds to settings-old.json");
}