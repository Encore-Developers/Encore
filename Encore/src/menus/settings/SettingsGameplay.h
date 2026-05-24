//
// Created by Jaydenz on 04/29/2025.
//

#ifndef SETTINGS_GAMEPLAY_H
#define SETTINGS_GAMEPLAY_H

#include <GLFW/glfw3.h>
#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonActionRegistry.h"
#include "menus/util/SettingRenderer.h"

namespace Encore {
    class SettingsGameplay {
    };
}

class SettingsGameplay : public OvershellMenu {
public:
    void Draw();
    void KeyboardInputCallback(int key, int scancode, int action, int mods);
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event);
    void Load();
    void Save();
    Encore::ButtonActionRegistry buttReg;
    bool ad = false;
    std::function<void()> ScanSongsFunc;
    Encore::SettingDoohickey settings;
private:
bool Fullscreen = false;
};

extern Encore::SettingsGameplay TheGameplaySettings;

#endif // SETTINGS_GAMEPLAY_H