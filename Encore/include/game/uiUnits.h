//
// Created by marie on 05/05/2024.
//

#ifndef ENCORE_UIUNITS_H
#define ENCORE_UIUNITS_H

#include <cmath>
#include "raylib.h"

class Units {
public:

    float RightBorder = ((float)GetScreenWidth()/2)+((float)GetScreenHeight()/1.25f);
    float RightSide = RightBorder >= (float)GetScreenWidth() ? (float)GetScreenWidth() : RightBorder;
    float LeftBorder = ((float)GetScreenWidth()/2)-((float)GetScreenHeight()/1.25f);
    float LeftSide = LeftBorder <= 0 ? 0 : LeftBorder;

    float wpct(float pct) {
        // for decimal points
        return (float)GetScreenHeight()*pct;
    }
    float wpct(int pct) {
        // for whole numbers
        return (float)GetScreenHeight()*((float)pct/100);
    }
    float hpct(float pct) {
        // for decimal points
        return (float)GetScreenHeight()*pct;
    }
    float hpct(int pct) {
        // for whole numbers
        return (float)GetScreenHeight()*((float)pct/100);
    }
};

#endif //ENCORE_UIUNITS_H
