//
// Created by Jaydenz on 04/29/2025.
//

#pragma once

#include "../menu.h"
#include "settings/settings.h"
#include "assets.h"
#include "../overshell/OvershellMenu.h"

#ifndef SETTINGSCREDITS_H
#define SETTINGSCREDITS_H



class SettingsCredits : public OvershellMenu {
#define OPTION(type, value, default) type value = default;
    SETTINGS_OPTIONS;
#undef OPTION
public:
    SettingsCredits() = default;
    ~SettingsCredits() override = default;
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) override;
    void Load() override;
    void Draw() override;
};


#endif //SETTINGSCREDITS_H
