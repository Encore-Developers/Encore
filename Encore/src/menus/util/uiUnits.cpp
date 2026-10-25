#include "uiUnits.h"
#include "raylib.h"
#include "raymath.h"

float Units::RightBorder;
float Units::RightSide;
float Units::LeftBorder;
float Units::LeftSide;

float Units::BottomBorder;
float Units::BottomSide;
float Units::TopBorder;
float Units::TopSide;

void Units::calcUnits() {
    RightBorder = ((float)GetRenderWidth() / 2) + ((float)GetRenderHeight() / 1.25f);
    RightSide =
        RightBorder >= (float)GetRenderWidth() ? (float)GetRenderWidth() : RightBorder;
    LeftBorder = ((float)GetRenderWidth() / 2) - ((float)GetRenderHeight() / 1.25f);
    LeftSide = LeftBorder <= 0 ? 0 : LeftBorder;
    BottomBorder = (GetRenderHeight() / 2) + (GetRenderWidth() / 3.0f);
    BottomSide = BottomBorder < GetRenderHeight() ? BottomBorder : GetRenderHeight();
    TopBorder = (GetRenderHeight() / 2) - ((GetRenderWidth() / 3.0f));
    TopSide = TopBorder < 0 ? 0 : TopBorder;
}

// usable height
float Units::hpct(float fpct) {
    // for decimal points
    // return ((BottomSide-TopSide)*fpct)+TopSide;
    return Remap(fpct, 0, 1.0f, TopSide, BottomSide);
}
float Units::hinpct(float fpct) {
    // for decimal points
    // return ((BottomSide-TopSide)*fpct)+TopSide;
    return Remap(fpct, 0, 1.0f, TopSide, BottomSide) - TopSide;
}

// usable width
float Units::wpct(float ipct) {
    // for decimal points
    return Remap(ipct, 0, 1.0f, LeftSide, RightSide);
}
float Units::winpct(float ipct) {
    // for decimal points
    return Remap(ipct, 0, 1.0f, LeftSide, RightSide) - LeftSide;
}
