//
// Created by marie on 03/09/2024.
//

#ifndef CACHELOADINGSCREEN_H
#define CACHELOADINGSCREEN_H
#include "menu.h"
#include "raylib.h"

class cacheLoadingScreen : public Menu {
    Texture2D encoreLogo;
    Font RedHatDisplay;
    Font RubikBold;
    Font JosefinSansItalic;
    int SplashSel;
    Texture2D LoadingScreenBackground;
    Shader sdfShader;

public:
    cacheLoadingScreen();
    virtual ~cacheLoadingScreen();
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override {};
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state) override {};
    void Draw() override;
    void Load() override;
};

#endif // CACHELOADINGSCREEN_H
