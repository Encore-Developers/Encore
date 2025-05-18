//
// Created by Jaydenz on 29/04/2025.
//

#ifndef SETTINGS_GAMEPLAY_H
#define SETTINGS_GAMEPLAY_H

#include <GLFW/glfw3.h>
#include "menu.h"
#include "settings.h"
#include "assets.h"
#include "OvershellMenu.h"

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


#endif // SETTINGS_GAMEPLAY_H