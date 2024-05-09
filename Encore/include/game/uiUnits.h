//
// Created by marie on 05/05/2024.
//

#ifndef ENCORE_UIUNITS_H
#define ENCORE_UIUNITS_H



class Units {
public:

float RightBorder;
float RightSide;
float LeftBorder;
float LeftSide;
float TopBorder;
float TopSide;
float BottomBorder;
float BottomSide;


    void calcUnits();
    // usable height
    float hpct(float pct);

    // usable width
    float wpct(float pct);
};

#endif //ENCORE_UIUNITS_H
