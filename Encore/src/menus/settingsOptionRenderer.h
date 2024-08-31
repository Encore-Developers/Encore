//
// Created by marie on 20/05/2024.
//

#ifndef ENCORE_SETTINGSOPTIONRENDERER_H
#define ENCORE_SETTINGSOPTIONRENDERER_H


#include <string>
#include "uiUnits.h"
#include "../assets.h"
#include "../keybinds.h"

class settingsOptionRenderer {
public:
    bool changingStrumUp = false;
    bool changingStrumDown = false;
    bool changingKey = false;
    bool changing4k = false;
    bool changingOverdrive = false;
    bool changingAlt = false;
    bool changingPause = false;
    int selLane = 0;
    float sliderEntry(float value, float min, float max, int entryNum, std::string Label, float increment);
    bool toggleEntry(bool value, int entryNum, std::string Label);
    void keybindEntryText(int entryNum, std::string Label);
    void keybind5kAltEntry(int altValue, int entryNum, std::string Label, Keybinds keybinds, int lane);
    void keybind5kEntry(int value, int entryNum, std::string Label, Keybinds keybinds, int lane);
    void keybind4kAltEntry(int altValue, int entryNum, std::string Label, Keybinds keybinds, int lane);
    void keybind4kEntry(int value, int entryNum, std::string Label, Keybinds keybinds, int lane);
    void keybindOdEntry(int value, int entryNum, std::string Label, Keybinds keybinds);
    void keybindOdAltEntry(int value, int entryNum, std::string Label, Keybinds keybinds);
    void keybindPauseEntry(int value, int entryNum, std::string Label, Keybinds keybinds);

    void keybindStrumEntry(int value, int entryNum, int key, Keybinds keybinds);
};


#endif //ENCORE_SETTINGSOPTIONRENDERER_H
