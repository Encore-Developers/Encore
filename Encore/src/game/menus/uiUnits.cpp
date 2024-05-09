#include "game/menus/uiUnits.h"
#include "raylib.h"
#include "raymath.h"

void Units::calcUnits() {
    RightBorder = ((float)GetScreenWidth()/2)+((float)GetScreenHeight()/1.25f);
    RightSide = RightBorder >= (float)GetScreenWidth() ? (float)GetScreenWidth() : RightBorder;
    LeftBorder = ((float)GetScreenWidth()/2)-((float)GetScreenHeight()/1.25f);
    LeftSide = LeftBorder <= 0 ? 0 : LeftBorder;
    BottomBorder = (GetScreenHeight() / 2) + (GetScreenWidth() / 3);
    BottomSide = BottomBorder < GetScreenHeight() ? BottomBorder : GetScreenHeight();
    TopBorder = (GetScreenHeight() / 2) - ((GetScreenWidth() / 3));
    TopSide = TopBorder < 0 ? 0 : TopBorder;
}
// usable height
float Units::hpct(float fpct) {
    // for decimal points
    calcUnits();
    // return ((BottomSide-TopSide)*fpct)+TopSide;
    return Remap(fpct, 0, 1.0f, TopSide, BottomSide);
}
float Units::hinpct(float fpct) {
    // for decimal points
    calcUnits();
    // return ((BottomSide-TopSide)*fpct)+TopSide;
    return Remap(fpct, 0, 1.0f, TopBorder, BottomBorder)-TopBorder;
}

// usable width
float Units::wpct(float ipct) {
    // for decimal points
    calcUnits();
    return Remap(ipct, 0, 1.0f, LeftSide, RightSide);
}
float Units::winpct(float ipct) {
    // for decimal points
    calcUnits();
    return Remap(ipct, 0, 1.0f, LeftBorder, RightBorder)-LeftBorder;
}