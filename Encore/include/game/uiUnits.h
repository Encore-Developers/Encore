//
// Created by marie on 05/05/2024.
//

#ifndef ENCORE_UIUNITS_H
#define ENCORE_UIUNITS_H

#include <cmath>
#include "raylib.h"

class Units {
public:
    static float default_pt;

    float rem = 16;
    static float pt(float point) {
        return GetWindowScaleDPI().x / (point * (0.0138888889f));
    };
    static float pt() {
        return GetWindowScaleDPI().x / (default_pt * (0.0138888889f));
    };
    float pica(float pica) {
        return GetWindowScaleDPI().x * (pica * (1/6));
    }
    float set_rem(float rem_in) {
        rem = GetWindowScaleDPI().x * (rem_in * (1/72));
    }
    static float window_percent(float pct) {
        // for decimal points
        return (float)GetScreenHeight()*pct;
    }
    static float window_percent(int pct) {
        // for whole numbers
        return (float)GetScreenHeight()*((float)pct/100);
    }
};

#endif //ENCORE_UIUNITS_H
