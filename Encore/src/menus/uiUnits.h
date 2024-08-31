//
// Created by marie on 05/05/2024.
//

#ifndef ENCORE_UIUNITS_H
#define ENCORE_UIUNITS_H

#include "raylib.h"

class Units {
    Units() {}
public:

    static Units& getInstance() {
        static Units instance; // This is the single instance
        return instance;
    }

float RightBorder;
float RightSide;
float LeftBorder;
float LeftSide;

float BottomBorder;
float BottomSide;
float TopBorder;
float TopSide;

    void calcUnits();
    // usable height
    float hpct(float pct);
    float hinpct(float pct);
    // usable width
    float wpct(float pct);
    float winpct(float pct);
};

#endif //ENCORE_UIUNITS_H
