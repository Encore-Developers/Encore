//
// Created by marie on 17/11/2024.
//

#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H
#include "menu.h"
#include "settings.h"

class SettingsMenu : public Menu {
#define OPTION(type, value, default) type value = default;
    SETTINGS_OPTIONS;
#undef OPTION
public:
    SettingsMenu() {};
    ~SettingsMenu() override {};
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state) override;
    void Load() override;
    void Draw() override;
};



#endif //SETTINGSMENU_H
