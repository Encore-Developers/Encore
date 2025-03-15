#pragma once
#include "GLFW/glfw3.h"

class Menu {
public:
    Menu() {}
    virtual ~Menu() {}

    virtual void KeyboardInputCallback(int key, int scancode, int action, int mods) = 0;
    virtual void ControllerInputCallback(int joypadID, GLFWgamepadstate state) = 0;

    virtual void Draw() = 0; // NOTE: requires BeginDrawing() to have already been called
    virtual void Load() = 0;
};

extern Menu *ActiveMenu;
