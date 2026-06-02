//
// Created by Jaydenz on 04/29/2025.
//

#pragma once

#include "../menu.h"
#include "settings/settings.h"
#include "assets.h"
#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonActionRegistry.h"
#include "menus/util/SettingRenderer.h"


class SettingsCredits : public OvershellMenu {
#define OPTION(type, value, default) type value = default;
    SETTINGS_OPTIONS;
#undef OPTION
    Encore::SettingDoohickey settings;
    Encore::ButtonActionRegistry buttReg;
    std::function<void()> maintainerFunc;
    std::function<void()> localeFunc;
    std::function<void()> founderFunc;
    std::function<void()> contributorFunc;
    std::vector<std::string> maintainer;
    std::vector<std::string> locale;
    std::vector<std::string> founder;
    std::vector<std::string> contributor;
public:
    SettingsCredits() = default;
    ~SettingsCredits() override = default;
    void KeyboardInputCallback(SDL_KeyboardEvent* event) override {};
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) override {};
    void Load() override;
    void Draw() override;
};
