#include "game/uiUnits.h"
#include "raylib.h"
    #include "raymath.h"

    void Units::calcUnits() {
        Units::RightBorder = ((float)GetScreenWidth()/2)+((float)GetScreenHeight()/1.25f);
        Units::RightSide = RightBorder >= (float)GetScreenWidth() ? (float)GetScreenWidth() : RightBorder;
        Units::LeftBorder = ((float)GetScreenWidth()/2)-((float)GetScreenHeight()/1.25f);
        Units::LeftSide = LeftBorder <= 0 ? 0 : LeftBorder;


        Units::BottomBorder = (GetScreenHeight() / 2) + (GetScreenWidth() / 3);
        Units::BottomSide = BottomBorder < GetScreenHeight() ? BottomBorder : GetScreenHeight();

        Units::TopBorder = (GetScreenHeight() / 2) - ((GetScreenWidth() / 3));
        Units::TopSide = TopBorder < 0 ? 0 : TopBorder;

    }

    // usable height
    float Units::hpct(float pct) {
        // for decimal points
        return ((BottomBorder-TopBorder)*pct)+TopBorder;
        // return Remap(pct, 0, 1.0f, TopSide, BottomSide);
    }

    // usable width
    float Units::wpct(float pct) {
        // for decimal points
        return ((RightSide-LeftSide)*pct)+LeftSide;
    }
