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

static const std::vector<std::string> presets = {
    "Thumb", "Thumb & Index", "Index & Middle"
};

void SettingsController::resetToDefaultKeys() {
    settings.controller4K = settings.defaultController4K;
    settings.controller5K = settings.defaultController5K;
    settings.controllerOverdrive = settings.defaultControllerOverdrive;
    settings.controllerPause = settings.defaultControllerPause;
    settings.controller4KAxisDirection = settings.defaultController4KAxisDirection;
    settings.controller5KAxisDirection = settings.defaultController5KAxisDirection;
    settings.controllerOverdriveAxisDirection = settings.defaultControllerOverdriveAxisDirection;
    settings.controllerPauseAxisDirection = settings.defaultControllerPauseAxisDirection;
    if (options.size() >= 11) {
        *options[0].second = settings.controller4K[0];
        *options[1].second = settings.controller4K[1];
        *options[2].second = settings.controller4K[2];
        *options[3].second = settings.controller4K[3];
        *options[4].second = settings.controller5K[0];
        *options[5].second = settings.controller5K[1];
        *options[6].second = settings.controller5K[2];
        *options[7].second = settings.controller5K[3];
        *options[8].second = settings.controller5K[4];
        *options[9].second = settings.controllerOverdrive;
        *options[10].second = settings.controllerPause;
    }
    settings.syncKeybindsToGame();
    TraceLog(LOG_INFO, "Reset controller bindings to defaults");
}

void SettingsController::applyPreset(int presetIndex) { // i fixed this in the most janky way possible
    switch (presetIndex) {
        case 0: // Thumb
            settings.controller4K = settings.defaultController4K;
            settings.controller5K = settings.defaultController5K;
            settings.controllerOverdrive = settings.defaultControllerOverdrive;
            settings.controllerPause = settings.defaultControllerPause;
            settings.controller4KAxisDirection = settings.defaultController4KAxisDirection;
            settings.controller5KAxisDirection = settings.defaultController5KAxisDirection;
            settings.controllerOverdriveAxisDirection = settings.defaultControllerOverdriveAxisDirection;
            settings.controllerPauseAxisDirection = settings.defaultControllerPauseAxisDirection;
            break;
        case 1: // Thumb & Index
            settings.controller4K = {-5, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_GAMEPAD_BUTTON_X, -6};
            settings.controller5K = {-5, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_Y, -6};
            settings.controllerOverdrive = GLFW_GAMEPAD_BUTTON_A;
            settings.controllerPause = GLFW_GAMEPAD_BUTTON_START;
            settings.controller4KAxisDirection = {1, 0, 0, 1};
            settings.controller5KAxisDirection = {1, 0, 0, 0, 1};
            settings.controllerOverdriveAxisDirection = 0;
            settings.controllerPauseAxisDirection = 0;
            break;
        case 2: // Index & Middle
            settings.controller4K = {-5, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, -6};
            settings.controller5K = {-5, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, -6};
            settings.controllerOverdrive = GLFW_GAMEPAD_BUTTON_A;
            settings.controllerPause = GLFW_GAMEPAD_BUTTON_START;
            settings.controller4KAxisDirection = {1, 0, 0, 1};
            settings.controller5KAxisDirection = {1, 0, 0, 0, 1};
            settings.controllerOverdriveAxisDirection = 0;
            settings.controllerPauseAxisDirection = 0;
            break;
        default:
            TraceLog(LOG_WARNING, "Invalid preset index: %d", presetIndex);
            return;
    }
    if (options.size() >= 11) {
        *options[0].second = settings.controller4K[0];
        *options[1].second = settings.controller4K[1];
        *options[2].second = settings.controller4K[2];
        *options[3].second = settings.controller4K[3];
        *options[4].second = settings.controller5K[0];
        *options[5].second = settings.controller5K[1];
        *options[6].second = settings.controller5K[2];
        *options[7].second = settings.controller5K[3];
        *options[8].second = settings.controller5K[4];
        *options[9].second = settings.controllerOverdrive;
        *options[10].second = settings.controllerPause;
    }
    settings.syncKeybindsToGame();
    TraceLog(LOG_INFO, "Applied preset: %s", presets[presetIndex].c_str());
}

void SettingsController::Draw() {
    static bool dropdownActive = false;
    static int selectedPreset = 0;
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
    float buttonWidth = SidebarWidth * 0.8f;
    float buttonHeight = u.hinpct(0.05f);
    float buttonSpacing = u.hinpct(0.02f);
    float bodyFontSize = u.hinpct(0.030f);
    float lineSpacing = bodyFontSize * 1.2f;
    std::vector<std::string> lines = split(sidebarBodyText, "\n");
    float bodyHeight = lines.size() * lineSpacing;
    float availableSpace = SidebarHeight - SidebarHeaderHeight - bodyHeight - u.hinpct(0.04f);
    float buttonAreaY = SidebarTop + SidebarHeaderHeight + (availableSpace - 2 * buttonHeight - buttonSpacing) / 2;

    Rectangle resetButtonRect = {
        SidebarLeft + (SidebarWidth - buttonWidth) / 2,
        buttonAreaY,
        buttonWidth,
        buttonHeight
    };
    if (GuiButton(resetButtonRect, "Reset to Defaults")) {
        resetToDefaultKeys();
        Save();
        TraceLog(LOG_INFO, "Reset controller bindings to defaults");
    }

    Rectangle dropdownRect = {
        SidebarLeft + (SidebarWidth - buttonWidth) / 2,
        buttonAreaY + buttonHeight + buttonSpacing,
        buttonWidth,
        buttonHeight
    };
    std::string presetList = "";
    for (size_t i = 0; i < presets.size(); ++i) {
        presetList += presets[i];
        if (i < presets.size() - 1) presetList += ";";
    }
    if (GuiDropdownBox(dropdownRect, presetList.c_str(), &selectedPreset, dropdownActive)) {
        dropdownActive = !dropdownActive;
        if (!dropdownActive && selectedPreset >= 0 && static_cast<size_t>(selectedPreset) < presets.size()) {
            applyPreset(selectedPreset);
            Save();
            TraceLog(LOG_INFO, "Applied preset: %s", presets[selectedPreset].c_str());
        }
    }

    float currentY = SidebarTop + SidebarHeaderHeight + availableSpace + u.hinpct(0.02f);
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
        int axisDirection = 0;
        if (label.find("4K Lane") != std::string::npos) {
            int laneIndex = std::stoi(label.substr(label.find_last_of(" ") + 1)) - 1;
            axisDirection = settings.controller4KAxisDirection[laneIndex];
        } else if (label.find("5K Lane") != std::string::npos) {
            int laneIndex = std::stoi(label.substr(label.find_last_of(" ") + 1)) - 1;
            axisDirection = settings.controller5KAxisDirection[laneIndex];
        } else if (label == "Overdrive") {
            axisDirection = settings.controllerOverdriveAxisDirection;
        } else if (label == "Pause") {
            axisDirection = settings.controllerPauseAxisDirection;
        }
        std::string buttonText = (*options[i].second == -2) ? "Unbound" :
            keybinds.getControllerStr(GLFW_JOYSTICK_1, *options[i].second, settings.controllerType, axisDirection);
        TraceLog(LOG_INFO, "Rendering controller bind %s: %s (code=%d)", label.c_str(), buttonText.c_str(), *options[i].second);
        if (static_cast<int>(i) == bindingOption) buttonText = "Press button...";
        if (CheckCollisionPointRec(mousePos, buttonRect)) {
            selectedIndex = i + 1;
            isHovering = true;
            DrawRectangleLinesEx(optionBoxRect, highlightBorderWidth, glowColor);
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                auto [bindType, bindIndex] = getBindTypeAndIndex(i);
                settings.rebindKey(bindType, bindIndex);
                if (i < options.size()) *options[i].second = -2;
                Save();
                TraceLog(LOG_INFO, "Unbound %s via right-click", label.c_str());
            }
        }
        if (GuiButton(buttonRect, buttonText.c_str())) {
            bindingOption = static_cast<int>(i);
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

std::pair<std::string, int> SettingsController::getBindTypeAndIndex(size_t optionIndex) {
    switch (optionIndex) {
        case 0: return {"controller4K", 0}; // 4K Lane 1
        case 1: return {"controller4K", 1}; // 4K Lane 2
        case 2: return {"controller4K", 2}; // 4K Lane 3
        case 3: return {"controller4K", 3}; // 4K Lane 4
        case 4: return {"controller5K", 0}; // 5K Lane 1
        case 5: return {"controller5K", 1}; // 5K Lane 2
        case 6: return {"controller5K", 2}; // 5K Lane 3
        case 7: return {"controller5K", 3}; // 5K Lane 4
        case 8: return {"controller5K", 4}; // 5K Lane 5
        case 9: return {"controllerOverdrive", 0}; // Overdrive
        case 10: return {"controllerPause", 0}; // Pause
    }
    return {"", -1};
}

void SettingsController::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    static bool dropdownActive = false;
    static int selectedPreset = 0;

    if (action != GLFW_PRESS) return;

    if (dropdownActive) {
        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_ESCAPE) {
            dropdownActive = false;
            if (key == GLFW_KEY_ENTER && selectedPreset >= 0 && static_cast<size_t>(selectedPreset) < presets.size()) {
                applyPreset(selectedPreset);
                Save();
                TraceLog(LOG_INFO, "Applied preset via keyboard: %s", presets[selectedPreset].c_str());
            }
            return;
        }
        if (key == GLFW_KEY_DOWN) {
            selectedPreset = (selectedPreset + 1) % presets.size();
        } else if (key == GLFW_KEY_UP) {
            selectedPreset = (selectedPreset - 1 + presets.size()) % presets.size();
        }
        return;
    }

    if (key == GLFW_KEY_DOWN) {
        selectedIndex = (selectedIndex + 1) % sidebarContents.size();
        if (selectedIndex == 0) selectedIndex = 1;
    } else if (key == GLFW_KEY_UP) {
        selectedIndex = (selectedIndex - 1 + sidebarContents.size()) % sidebarContents.size();
        if (selectedIndex == 0) selectedIndex = sidebarContents.size() - 1;
    } else if (key == GLFW_KEY_R) {
        resetToDefaultKeys();
        Save();
        TraceLog(LOG_INFO, "Reset controller bindings");
    } else if (key == GLFW_KEY_ENTER) {
        if (selectedIndex > 0) bindingOption = selectedIndex - 1;
        else dropdownActive = true;
    } else if (key == GLFW_KEY_ESCAPE) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
}

void SettingsController::ControllerInputCallback(int joypadID, GLFWgamepadstate state) {
    static GLFWgamepadstate prevState;
    static std::vector<float> debounceTimers(GLFW_GAMEPAD_AXIS_LAST + 1, 0.0f);
    const float debounceTime = 0.2f;
    float deltaTime = GetFrameTime();

    if (bindingOption >= 0 && bindingOption < static_cast<int>(options.size())) {
        auto [bindType, bindIndex] = getBindTypeAndIndex(bindingOption);
        if (!bindType.empty()) {
            for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i) {
                if (state.buttons[i] && !prevState.buttons[i]) {
                    *options[bindingOption].second = i;
                    if (bindType == "controller4K" && bindIndex < 4) {
                        settings.controller4K[bindIndex] = i;
                        settings.controller4KAxisDirection[bindIndex] = 0;
                    } else if (bindType == "controller5K" && bindIndex < 5) {
                        settings.controller5K[bindIndex] = i;
                        settings.controller5KAxisDirection[bindIndex] = 0;
                    } else if (bindType == "controllerOverdrive") {
                        settings.controllerOverdrive = i;
                        settings.controllerOverdriveAxisDirection = 0;
                    } else if (bindType == "controllerPause") {
                        settings.controllerPause = i;
                        settings.controllerPauseAxisDirection = 0;
                    }
                    settings.syncKeybindsToGame();
                    TraceLog(LOG_INFO, "Bound %s to button %d (%s)",
                             options[bindingOption].first.c_str(), i,
                             keybinds.getControllerStr(joypadID, i, settings.controllerType, 0).c_str());
                    bindingOption = -1;
                    Save();
                    prevState = state;
                    return;
                }
            }
            for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; ++i) {
                float value = state.axes[i];
                float prevValue = prevState.axes[i];
                if (fabs(value) > 0.6f && fabs(prevValue) <= 0.6f && debounceTimers[i] <= 0.0f) {
                    int axisCode = -1 - i;
                    *options[bindingOption].second = axisCode;
                    int direction = (value > 0) ? 1 : -1;
                    if (bindType == "controller4K" && bindIndex < 4) {
                        settings.controller4K[bindIndex] = axisCode;
                        settings.controller4KAxisDirection[bindIndex] = direction;
                    } else if (bindType == "controller5K" && bindIndex < 5) {
                        settings.controller5K[bindIndex] = axisCode;
                        settings.controller5KAxisDirection[bindIndex] = direction;
                    } else if (bindType == "controllerOverdrive") {
                        settings.controllerOverdrive = axisCode;
                        settings.controllerOverdriveAxisDirection = direction;
                    } else if (bindType == "controllerPause") {
                        settings.controllerPause = axisCode;
                        settings.controllerPauseAxisDirection = direction;
                    }
                    settings.syncKeybindsToGame();
                    TraceLog(LOG_INFO, "Bound %s to axis %d direction %s",
                             options[bindingOption].first.c_str(), i,
                             (value > 0 ? "+" : "-"));
                    bindingOption = -1;
                    debounceTimers[i] = debounceTime;
                    Save();
                    prevState = state;
                    return;
                }
                debounceTimers[i] = std::max(0.0f, debounceTimers[i] - deltaTime);
            }
        }
    }
    if (bindingOption < 0 && state.buttons[GLFW_GAMEPAD_BUTTON_B] && !prevState.buttons[GLFW_GAMEPAD_BUTTON_B]) {
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
    TraceLog(LOG_INFO, "5K Controller: [%d, %d, %d, %d, %d]",
             settings.controller5K[0], settings.controller5K[1],
             settings.controller5K[2], settings.controller5K[3],
             settings.controller5K[4]);
    TraceLog(LOG_INFO, "Overdrive: %d, Pause: %d",
             settings.controllerOverdrive, settings.controllerPause);

    options.clear();
    options.emplace_back("4K Lane 1", &settings.controller4K[0]);
    options.emplace_back("4K Lane 2", &settings.controller4K[1]);
    options.emplace_back("4K Lane 3", &settings.controller4K[2]);
    options.emplace_back("4K Lane 4", &settings.controller4K[3]);
    options.emplace_back("5K Lane 1", &settings.controller5K[0]);
    options.emplace_back("5K Lane 2", &settings.controller5K[1]);
    options.emplace_back("5K Lane 3", &settings.controller5K[2]);
    options.emplace_back("5K Lane 4", &settings.controller5K[3]);
    options.emplace_back("5K Lane 5", &settings.controller5K[4]);
    options.emplace_back("Overdrive", &settings.controllerOverdrive);
    options.emplace_back("Pause", &settings.controllerPause);

    sidebarContents = {
        {"Controller Bindings", "Configure your controller bindings"},
        {"4K Lane 1", "TBA"},
        {"4K Lane 2", "TBA"},
        {"4K Lane 3", "TBA"},
        {"4K Lane 4", "TBA"},
        {"5K Lane 1", "TBA"},
        {"5K Lane 2", "TBA"},
        {"5K Lane 3", "TBA"},
        {"5K Lane 4", "TBA"},
        {"5K Lane 5", "TBA"},
        {"Overdrive", "TBA"},
        {"Pause", "TBA"}
    };
}

void SettingsController::Save() {
    settings.saveOldSettings(settings.getDirectory() / "settings-old.json");
    settings.syncKeybindsToGame();
    TraceLog(LOG_INFO, "SettingsController: Saved controller binds to settings-old.json");
}