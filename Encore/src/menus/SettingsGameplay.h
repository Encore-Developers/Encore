//
// Created by Jaydenz on 04/29/2025.
//

#ifndef SETTINGS_GAMEPLAY_H
#define SETTINGS_GAMEPLAY_H

#include <GLFW/glfw3.h>
#include "menu.h"

namespace Encore {
    class SettingsGameplay : public Menu {
    public:
        void Draw();
        void KeyboardInputCallback(int key, int scancode, int action, int mods);
        void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event);
        void Load();
        void Save();

    private:
    bool Fullscreen = false;
    };
}


#endif // SETTINGS_GAMEPLAY_H