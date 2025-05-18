//
// Created by Jaydenz on 04/29/2025.
//
#pragma once

#include "menu.h"
#include "settings.h"
#include "assets.h"
#include "OvershellMenu.h"

#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H

class SettingsController : public OvershellMenu {
#define OPTION(type, value, default) type value = default;
    SETTINGS_OPTIONS;
#undef OPTION
public:
    SettingsController() = default;
    ~SettingsController() override = default;
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state) override;
    void Load() override;
    void Draw() override;
};

#endif //SETTINGSCONTROLLER_H
