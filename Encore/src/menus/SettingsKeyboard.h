//
// Created by Jaydenz on 04/29/2025.
//

#pragma once

#include "OvershellMenu.h"
#include "settings-old.h"
#include "keybinds.h"
#include "../../include/assets.h"
#include "settings.h"

#ifndef SETTINGSKEYBOARD_H

namespace Encore {
    class SettingsKeyboard {
    };
}

class SettingsKeyboard : public OvershellMenu {
#define OPTION(type, value, default) type value = default;
#undef OPTION
public:
    SettingsKeyboard() = default;
    ~SettingsKeyboard() override = default;
    void Draw() override;
    static std::pair<std::string, int> getBindTypeAndIndex(size_t optionIndex);
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state) override;
    void Load();
    void Save();

private:
    SettingsOld& settings = SettingsOld::getInstance();
    Keybinds keybinds;
    int selectedIndex = 0;
    int bindingOption = -1;
    bool isHovering = false;
    const float boxWidthPct = 0.55f;
    std::vector<std::pair<std::string, int*>> options = {
        {"4K Lane 1", &settings.keybinds4K[0]},
        {"4K Lane 2", &settings.keybinds4K[1]},
        {"4K Lane 3", &settings.keybinds4K[2]},
        {"4K Lane 4", &settings.keybinds4K[3]},
        {"5K Lane 1", &settings.keybinds5K[0]},
        {"5K Lane 2", &settings.keybinds5K[1]},
        {"5K Lane 3", &settings.keybinds5K[2]},
        {"5K Lane 4", &settings.keybinds5K[3]},
        {"5K Lane 5", &settings.keybinds5K[4]},
        {"Overdrive", &settings.keybindOverdrive},
        {"Overdrive Alt", &settings.keybindOverdriveAlt},
        {"Pause", &settings.keybindPause}
    };
    struct SidebarContent {
        const char* header;
        const char* body;
    };
    std::vector<SidebarContent> sidebarContents = {
        {"Keyboard Bindings", "TBD"},
        {"4K Lane 1", "TBD"},
        {"4K Lane 2", "TBD"},
        {"4K Lane 3", "TBD"},
        {"4K Lane 4", "TBD"},
        {"5K Lane 1", "TBD"},
        {"5K Lane 2", "TBD"},
        {"5K Lane 3", "TBD"},
        {"5K Lane 4", "TBD"},
        {"5K Lane 5", "TBD"},
        {"Overdrive", "TBD"},
        {"Overdrive Alt", "TBD"},
        {"Pause", "TBD"}
    };
};

extern Encore::SettingsKeyboard TheKeyboardSettings;

#endif