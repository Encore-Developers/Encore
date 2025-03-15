#pragma once

#include <vector>
#include <string>

#include "raylib.h"

#include "menu.h"

class SoundTestMenu : public Menu {
    std::vector<std::string> mSoundIds;
    Font mFont;

public:
    SoundTestMenu();
    virtual ~SoundTestMenu();
    virtual void KeyboardInputCallback(int key, int scancode, int action, int mods) {};
    virtual void ControllerInputCallback(int joypadID, GLFWgamepadstate state) {};
    virtual void Load();
    virtual void Draw();
};
