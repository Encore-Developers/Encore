//
// Created by marie on 03/08/2024.
//

#ifndef OVERSHELLRENDERER_H
#define OVERSHELLRENDERER_H
#include "users/player.h"

class OvershellRenderer {
public:
    int AvailableControllers = 0; // keyb is always available
    void DrawTopOvershell(double height);
    void DrawBottomOvershell();
    bool CanMouseClick = true;
};

#endif // OVERSHELLRENDERER_H
