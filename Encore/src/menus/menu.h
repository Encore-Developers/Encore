#pragma once
#include "GLFW/glfw3.h"
#include "RhythmEngine/REenums.h"

class Menu {
public:
    bool UIInput = true;

    Menu() {}
    virtual ~Menu() {}

    virtual void KeyboardInputCallback(int key, int scancode, int action, int mods) = 0;
    virtual void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) = 0;

    virtual void Draw() = 0; // NOTE: requires BeginDrawing() to have already been called
    virtual void Load() = 0;
};

extern Menu *ActiveMenu;
