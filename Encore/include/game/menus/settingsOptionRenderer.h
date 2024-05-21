//
// Created by marie on 20/05/2024.
//

#ifndef ENCORE_SETTINGSOPTIONRENDERER_H
#define ENCORE_SETTINGSOPTIONRENDERER_H


#include <string>
#include "uiUnits.h"
#include "game/assets.h"
#include "game/keybinds.h"

class settingsOptionRenderer {
public:

    bool changingKey = false;
    bool changing4k = false;
    bool changingOverdrive = false;
    bool changingAlt = false;
    bool changingPause = false;
    int selLane = 0;
    float sliderEntry(float value, float min, float max, int entryNum, std::string Label, float increment, Assets assets);
    bool toggleEntry(bool value, int entryNum, std::string Label,  Assets assets);
    void keybindEntryText(int entryNum, std::string Label, Assets assets);
    void keybindAltEntry(int altValue, int entryNum, std::string Label,  Assets assets, Keybinds keybinds, int lane);
    void keybindEntry(int value, int entryNum, std::string Label,  Assets assets, Keybinds keybinds, int lane);
    void keybindOdEntry(int value, int entryNum, std::string Label,  Assets assets, Keybinds keybinds, int lane);
    void keybindOdAltEntry(int value, int entryNum, std::string Label,  Assets assets, Keybinds keybinds, int lane);
};


#endif //ENCORE_SETTINGSOPTIONRENDERER_H
