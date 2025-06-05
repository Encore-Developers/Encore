//
// Created by Jaydenz on 04/29/2025.
//

#pragma once

#include "menu.h"
#include "settings.h"
#include "../../include/assets.h"
#include "OvershellMenu.h"

#ifndef SETTINGSCREDITS_H
#define SETTINGSCREDITS_H

namespace Encore {
    class SettingsCredits {
    };
}

class SettingsCredits : public OvershellMenu {
#define OPTION(type, value, default) type value = default;
    SETTINGS_OPTIONS;
#undef OPTION
public:
    SettingsCredits() = default;
    ~SettingsCredits() override = default;
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state) override;
    void Load() override;
    void Draw() override;
};

extern Encore::SettingsCredits TheCredits;

#endif //SETTINGSCREDITS_H
