//
// Created by Jaydenz on 04/29/2025.
//

#ifndef SETTINGS_GAMEPLAY_H
#define SETTINGS_GAMEPLAY_H

#include <GLFW/glfw3.h>
#include "OvershellMenu.h"

namespace Encore {
    class SettingsGameplay {
    };
}

class SettingsGameplay : public OvershellMenu {
public:
    void Draw();
    void KeyboardInputCallback(int key, int scancode, int action, int mods);
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state);
    void Load();
    void Save();

private:
bool Fullscreen = false;
};

extern Encore::SettingsGameplay TheGameplaySettings;

#endif // SETTINGS_GAMEPLAY_H