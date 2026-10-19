#pragma once
#include "raylib.h"

class Units {
public:
    static float RightBorder;
    static float RightSide;
    static float LeftBorder;
    static float LeftSide;

    static float BottomBorder;
    static float BottomSide;
    static float TopBorder;
    static float TopSide;

    static void calcUnits();
    // usable height
    static float hpct(float pct);
    static float hinpct(float pct);
    // usable width
    static float wpct(float pct);
    static float winpct(float pct);
};