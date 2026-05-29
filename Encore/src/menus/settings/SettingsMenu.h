//
// Created by marie on 17/11/2024.
//

#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H
#include "../menu.h"
#include "settings/settings.h"
#include "assets.h"
#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonActionRegistry.h"


class SettingsMenu : public OvershellMenu {
    Encore::ButtonActionRegistry buttReg;

    std::array<std::string, 5> menuItems = {
        "settings.header.audioVisual",
        "settings.header.gameplay",
        "settings.header.controller",
        "settings.header.keyboard",
        "settings.header.credits"
    };
public:

    SettingsMenu() = default;
    ~SettingsMenu() override = default;
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) override;
    void Load() override;
    void Draw() override;
};

#endif //SETTINGSMENU_H
