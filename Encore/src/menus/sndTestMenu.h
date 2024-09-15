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

    virtual void Draw();
    virtual void Load();
};
