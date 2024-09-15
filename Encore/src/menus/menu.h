#pragma once

class Menu {
public:
    Menu() {}
    virtual ~Menu() {}

    virtual void Draw() = 0; // NOTE: requires BeginDrawing() to have already been called
    virtual void Load() = 0;
    static bool onNewMenu;
};

extern Menu *ActiveMenu;
