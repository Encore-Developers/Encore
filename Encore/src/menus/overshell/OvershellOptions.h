//
// Created by maria on 09/01/2025.
//
/*
#ifndef OVERSHELLOPTIONS_H
#define OVERSHELLOPTIONS_H
#include "assets.h"
#include "raygui.h"
#include "menus/uiUnits.h"

#include <cmath>

template <typename T>
class OvershellOption {
public:
    T &value;
    std::string name;
    OvershellOption();
    // so unfortunately, Draw handles click action and other stupid shit
    // raygui. i love you. but i hate you
    virtual void Draw(int pos, int slot);
};

class OvershellSlider : public OvershellOption<float> {
public:
    float step;
    float min;
    float max;
    bool modifying;
    OvershellSlider(std::string _name, float &_value, float _step, float _min, float _max) {
        name = _name;
        value = _value;
        step = _step;
        min = _min;
        max = _max;
    };
    void Draw(int pos, int slot) override {
        Units &unit = Units::getInstance();
        float OvershellLeftLoc =
            (unit.wpct(0.125) + (unit.winpct(0.25) * slot)) - unit.winpct(0.1);
        float height = unit.winpct(0.03f);
        float widthNoHeight = unit.winpct(0.2f) - height;
        Rectangle bounds = { OvershellLeftLoc + height,
                             unit.hpct(1.0f) - (unit.winpct(0.03f) * (pos + 1)),
                             unit.winpct(0.2f) - height - height,
                             height };
        Rectangle confirmBounds = { OvershellLeftLoc + widthNoHeight,
                                    unit.hpct(1.0f) - (unit.winpct(0.03f) * (pos + 1)),
                                    height,
                                    height };
        Assets &assets = Assets::getInstance();
        if (modifying) {
            GuiSlider(bounds, "", "", &value, min, max);
            GuiButton(
                { OvershellLeftLoc,
                  unit.hpct(1.0f) - (unit.winpct(0.03f) * (pos + 1)),
                  height,
                  height },
                TextFormat("%1.1f", value)
            );
            value = (round(value / step) * step);

            if (GuiButton(confirmBounds, "<")) {
                modifying = false;
            };
        } else {
            if (GuiButton(
                    { OvershellLeftLoc,
                      unit.hpct(1.0f) - (unit.winpct(0.03f) * (pos + 1)),
                      unit.winpct(0.2f),
                      unit.winpct(0.03f) },
                    name.c_str()
                )) {
                modifying = true;
            }
        }
    };
};

class OvershellToggle : public OvershellOption<bool> {
public:
    OvershellToggle(bool &ValueToChange) { value = ValueToChange; };
    void Draw(int pos, int slot) override {
        Units &unit = Units::getInstance();
        float OvershellLeftLoc =
            (unit.wpct(0.125) + (unit.winpct(0.25) * slot)) - unit.winpct(0.1);
        float height = unit.winpct(0.03f);
        float widthNoHeight = unit.winpct(0.2f);
        Rectangle bounds = { OvershellLeftLoc + height,
                             unit.hpct(1.0f) - (unit.winpct(0.03f) * (pos + 1)),
                             unit.winpct(0.2f) - height - height,
                             height };
        Rectangle confirmBounds = { OvershellLeftLoc + widthNoHeight,
                                    unit.hpct(1.0f) - (unit.winpct(0.03f) * (pos + 1)),
                                    height,
                                    height };
        Assets &assets = Assets::getInstance();

        if (GuiButton(
                { OvershellLeftLoc,
                  unit.hpct(1.0f) - (unit.winpct(0.03f) * (pos + 1)),
                  widthNoHeight,
                  height },
                name.c_str()
            )) {
            value = !value;
            }

        DrawRectanglePro(confirmBounds, { 0 }, 0, value ? GREEN : RED);
    };
};

// when i actually do this. it should allow you to make it "click" to add to the
// overshell stack
class OvershellButton : public OvershellOption<void> {
public:
    OvershellButton() {};
};

void thing() {
    float Piss = 0;
    bool Dong = false;
    OvershellSlider option("Urinate", Piss, 0.1, 0, 5);
    OvershellToggle toggle(Dong);
}

#endif // OVERSHELLOPTIONS_H
*/