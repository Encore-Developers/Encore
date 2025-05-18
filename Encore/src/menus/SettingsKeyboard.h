//
// Created by Jaydenz on 29/04/2025.
//

#pragma once

#include "menu.h"
#include "settings.h"
#include "assets.h"
#include "OvershellMenu.h"

#ifndef SETTINGSKEYBOARD_H
#define SETTINGSKEYBOARD_H

class SettingsKeyboard : public OvershellMenu {
#define OPTION(type, value, default) type value = default;
    SETTINGS_OPTIONS;
#undef OPTION
public:
    SettingsKeyboard() = default;
    ~SettingsKeyboard() override = default;
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state) override;
    void Load() override;
    void Draw() override;
};

#endif //SETTINGSKEYBOARD_H
