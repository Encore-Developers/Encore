//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsController.h"

#include "SettingsMenu.h"
#include "../MenuManager.h"
#include "../main/MainMenu.h"
#include "assets.h"
#include "raygui.h"
#include "../util/uiUnits.h"
#include "util/settings-text.h"
#include "../overshell/OvershellHelper.h"
#include "menus/util/locale/Locale.h"

static const std::vector<std::string> presets = {
    "Thumb", "Thumb & Index", "Index & Middle"
};
// TODO: fuck my ass
void SettingsController::resetToDefaultKeys() {
    /*
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
    */
}

void SettingsController::applyPreset(int presetIndex) { // i fixed this in the most janky way possible
    /*
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
    */
}

void SettingsController::Draw() {

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

void SettingsController::KeyboardInputCallback(SDL_KeyboardEvent* event) {

}

void SettingsController::ControllerInputCallback(Encore::ControllerEvent event) {
    if (event.action == Encore::Action::PRESS) {
        switch (event.channel) {
        case Encore::InputChannel::LANE_2: {
            Save();
            TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
            break;
        }
        }
    }
    /*
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
    */
}

void SettingsController::Load() {
    /*
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
*/
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
    // settings.saveOldSettings(settings.getDirectory() / "settings-old.json");
    // settings.syncKeybindsToGame();
    // Encore::Log::Debug("Saved controller binds to settings-old.json");
}