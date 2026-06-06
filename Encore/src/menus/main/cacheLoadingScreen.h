//
// Created by marie on 03/09/2024.
//

#ifndef CACHELOADINGSCREEN_H
#define CACHELOADINGSCREEN_H
#include "../menu.h"
#include "raylib.h"

class cacheLoadingScreen : public Menu {
    int SplashSel;

public:
    cacheLoadingScreen();
    virtual ~cacheLoadingScreen();
    void KeyboardInputCallback(SDL_KeyboardEvent* event) override {};
    void ControllerInputCallback(Encore::ControllerEvent event) override {};
    void Draw() override;
    void Load() override;
};

#endif // CACHELOADINGSCREEN_H
