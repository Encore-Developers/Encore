#pragma once
#include <string>
#include "raylib.h"

namespace encOS {
    bool OvershellButton(int slot, int x, std::string string);

    bool OvershellSlider(
        int slot, int x, std::string string, float *value, float step, float min, float max
    );
    void DrawBeacon(int slot, float x, float y, float width, float height, bool top, Color playerColor);
    void DrawTopOvershell(double height);
    bool DrawOvershellRectangleHeader(
        float x,
        float y,
        float width,
        float height,
        std::string username,
        Color accentColor,
        Color usernameColor
    );

    bool OvershellCheckbox(int slot, int x, std::string string, bool initialVal);
}
