//
// Created by marie on 20/05/2024.
//

#include "game/menus/settingsOptionRenderer.h"
#include "game/menus/uiUnits.h"
#include "game/assets.h"
#include "raygui.h"

std::string trFloatString(float& input) {
    std::string inputStr = std::to_string(input);
    size_t dotPos = inputStr.find('.');
    if (dotPos != std::string::npos && dotPos + 3 < inputStr.length()) {
        return inputStr.substr(0,dotPos + 3);
    }
    return "";
}

Units units = Units::getInstance();
Assets &soreAss = Assets::getInstance();

float width = 0.989f;

float settingsOptionRenderer::sliderEntry(float value, float min, float max, int entryNum, std::string Label, float increment) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float lengthTop = EntryTop + (EntryHeight * (entryNum-1));
    float lengthTextTop = EntryTextTop + (EntryHeight * (entryNum-1));
    float lengthFloat = value;
    DrawTextEx(soreAss.rubikBold, Label.c_str(), {EntryTextLeft, lengthTextTop}, EntryFontSize, 0, WHITE );
    // main slider

    if (GuiSliderBar({ OptionLeft+EntryHeight, lengthTop,OptionWidth-(EntryHeight * 2),EntryHeight }, "", "", &lengthFloat, min, max)) {
        value = lengthFloat;
    }
    // slider side buttons
    if (GuiButton({ OptionLeft,lengthTop,EntryHeight,EntryHeight }, "<")) {
        value-=increment;
    }
    if (GuiButton({ OptionRight - EntryHeight ,lengthTop,EntryHeight,EntryHeight }, ">")) {
        value+=increment;
    }
    float ValueMiddle = MeasureTextEx(soreAss.rubikBold, trFloatString(value).c_str(), EntryFontSize, 0).x / 2;
    DrawTextEx(soreAss.rubikBold,trFloatString(value).c_str(), {OptionRight - (OptionWidth /2) -ValueMiddle, lengthTextTop}, EntryFontSize, 0, WHITE);
    return value;
};

bool settingsOptionRenderer::toggleEntry(bool value, int entryNum, std::string Label){
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float valueTop = EntryTop + (EntryHeight * (entryNum-1));
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum-1));
    DrawTextEx(soreAss.rubikBold, Label.c_str(), {EntryTextLeft, valueTextTop}, EntryFontSize, 0, WHITE );
    // main slider
    if (GuiButton({ OptionLeft, valueTop,OptionWidth,EntryHeight }, TextFormat("%s", value ? "On" : "Off"))) {
        value= !value;
    }
    return value;
};

void settingsOptionRenderer::keybind5kAltEntry(int altValue, int entryNum, std::string Label,
                                             Keybinds keybinds, int lane) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float valueTop = EntryTop + (EntryHeight * (entryNum-1));
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum-1));

    if (GuiButton({ OptionLeft+(OptionWidth/2),valueTop,OptionWidth/2,EntryHeight  }, keybinds.getKeyStr(altValue).c_str())) {
        changing4k = false;
        changingAlt = true;
        selLane = lane;
        changingKey = true;
    }
}


void settingsOptionRenderer::keybind5kEntry(int value, int entryNum, std::string Label,
                                          Keybinds keybinds, int lane) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float valueTop = EntryTop + (EntryHeight * (entryNum - 1));
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum - 1));

    if (GuiButton({OptionLeft, valueTop, OptionWidth / 2, EntryHeight}, keybinds.getKeyStr(value).c_str())) {
        changing4k = false;
        changingAlt = false;
        selLane = lane;
        changingKey = true;
    }
}

void settingsOptionRenderer::keybind4kAltEntry(int altValue, int entryNum, std::string Label,
                                               Keybinds keybinds, int lane) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float valueTop = EntryTop + (EntryHeight * (entryNum-1));
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum-1));

    if (GuiButton({ OptionLeft+(OptionWidth/2),valueTop,OptionWidth/2,EntryHeight  }, keybinds.getKeyStr(altValue).c_str())) {
        changing4k = true;
        changingAlt = true;
        selLane = lane;
        changingKey = true;
    }
}


void settingsOptionRenderer::keybind4kEntry(int value, int entryNum, std::string Label,
                                            Keybinds keybinds, int lane) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float valueTop = EntryTop + (EntryHeight * (entryNum - 1));
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum - 1));

    if (GuiButton({OptionLeft, valueTop, OptionWidth / 2, EntryHeight}, keybinds.getKeyStr(value).c_str())) {
        changing4k = true;
        changingAlt = false;
        selLane = lane;
        changingKey = true;
    }
}

void settingsOptionRenderer::keybindOdAltEntry(int altValue, int entryNum, std::string Label,
                                               Keybinds keybinds) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float valueTop = EntryTop + (EntryHeight * (entryNum-1));
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum-1));

    if (GuiButton({ OptionLeft+(OptionWidth/2),valueTop,OptionWidth/2,EntryHeight  }, keybinds.getKeyStr(altValue).c_str())) {
        changingAlt = true;
        changingKey = false;
        changingOverdrive = true;
    }
}


void settingsOptionRenderer::keybindOdEntry(int value, int entryNum, std::string Label,
                                            Keybinds keybinds) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float valueTop = EntryTop + (EntryHeight * (entryNum - 1));
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum - 1));

    if (GuiButton({OptionLeft, valueTop, OptionWidth / 2, EntryHeight}, keybinds.getKeyStr(value).c_str())) {
        changingAlt = false;
        changingKey = false;
        changingOverdrive = true;
    }
}

void settingsOptionRenderer::keybindPauseEntry(int value, int entryNum, std::string Label,
                                            Keybinds keybinds) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float valueTop = EntryTop + (EntryHeight * (entryNum - 1));
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum - 1));

    if (GuiButton({OptionLeft, valueTop, OptionWidth / 2, EntryHeight}, keybinds.getKeyStr(value).c_str())) {
        changingPause = true;
    }
}

void settingsOptionRenderer::keybindStrumEntry(int value, int entryNum, int key,
                                               Keybinds keybinds) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;

    float valueTop = EntryTop + (EntryHeight * (entryNum - 1));
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum - 1));

    if (GuiButton({OptionLeft, valueTop, OptionWidth / 2, EntryHeight}, keybinds.getKeyStr(key).c_str())) {
        if (value == 0) {
            changingStrumUp = true;
        } else if (value == 1) {
            changingStrumDown = true;
        }
    }
}




void settingsOptionRenderer::keybindEntryText(int entryNum, std::string Label) {
    float OvershellBottom = units.hpct(0.15f);
    float EntryFontSize = units.hinpct(0.03f);
    float EntryHeight = units.hinpct(0.05f);
    float EntryTop = OvershellBottom + units.hinpct(0.1f);
    float EntryTextLeft = units.LeftSide + units.winpct(0.025f);
    float EntryTextTop = EntryTop + units.hinpct(0.01f);
    float OptionLeft = units.LeftSide+units.winpct(0.005f)+(units.winpct(width) / 3);
    float OptionWidth = units.winpct(width) / 3;
    float OptionRight = OptionLeft + OptionWidth;
    float valueTextTop = EntryTextTop + (EntryHeight * (entryNum - 1));

    DrawTextEx(soreAss.rubikBold, Label.c_str(), {EntryTextLeft, valueTextTop}, EntryFontSize, 0, WHITE );
}
