//
// Created by Jaydenz on 04/29/2025.
//
#pragma once

#include "OvershellMenu.h"
#include "settings-old.h"
#include "keybinds.h"
#include <string>
#include <utility>
#include <vector>

#ifndef SETTINGSCONTROLLER_H

namespace Encore {
    class SettingsController {
    };
}

class SettingsController : public OvershellMenu {
public:
    SettingsController() = default;
    ~SettingsController() override = default;
    void resetToDefaultKeys();
    void applyPreset(int presetIndex);
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
    bool dropdownActive = false;
    int selectedPreset = 0;
    bool isHovering = false;
    const float boxWidthPct = 0.55f;
    std::vector<std::pair<std::string, int*>> options = {
        {"4K Lane 1", &settings.controller4K[0]},
        {"4K Lane 2", &settings.controller4K[1]},
        {"4K Lane 3", &settings.controller4K[2]},
        {"4K Lane 4", &settings.controller4K[3]},
        {"5K Lane 1", &settings.controller5K[0]},
        {"5K Lane 2", &settings.controller5K[1]},
        {"5K Lane 3", &settings.controller5K[2]},
        {"5K Lane 4", &settings.controller5K[3]},
        {"5K Lane 5", &settings.controller5K[4]},
        {"Overdrive", &settings.controllerOverdrive},
        {"Pause", &settings.controllerPause}
    };
    struct SidebarContent {
        const char* header;
        const char* body;
    };
    std::vector<SidebarContent> sidebarContents = {
        {"Controller Bindings", "TBA"},
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
};

extern Encore::SettingsController TheControllerSettings;

#endif