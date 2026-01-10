//
// Created by marie on 03/09/2024.
//

#ifndef CACHELOADINGSCREEN_H
#define CACHELOADINGSCREEN_H
#include "menu.h"
#include "raylib.h"

class cacheLoadingScreen : public Menu {
    int SplashSel;

public:
    cacheLoadingScreen();
    virtual ~cacheLoadingScreen();
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override {};
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state) override {};
    void Draw() override;
    void Load() override;
};

#endif // CACHELOADINGSCREEN_H
