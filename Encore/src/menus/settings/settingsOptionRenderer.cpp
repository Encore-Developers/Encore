//
// Created by marie on 20/05/2024.
//

#include "settingsOptionRenderer.h"
#include "../util/uiUnits.h"
#include "assets.h"
#include "raygui.h"

// will update when other menus are made
#include <string>
#include <functional>
#include <sstream>
#include <iomanip>

std::string trFloatString(float input) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << input;
    return ss.str();
}


static constexpr float WIDTH = 0.989f;

static float truncateFloatString(float value) {
    std::string str = std::to_string(value);
    size_t dotPos = str.find('.');
    if (dotPos != std::string::npos && dotPos + 3 < str.length()) {
        return std::stof(str.substr(0, dotPos + 3));
    }
    return value;
}

struct Layout {
    float overshellBottom;
    float entryFontSize;
    float entryHeight;
    float entryTop;
    float entryTextLeft;
    float entryTextTop;
    float optionLeft;
    float optionWidth;
    float optionRight;

    Layout() {
        overshellBottom = Units::hpct(0.15f);
        entryFontSize = Units::hinpct(0.03f);
        entryHeight = Units::hinpct(0.05f);
        entryTop = overshellBottom + Units::hinpct(0.1f);
        entryTextLeft = Units::LeftSide + Units::winpct(0.025f);
        entryTextTop = entryTop + Units::hinpct(0.01f);
        optionLeft = Units::LeftSide + Units::winpct(0.005f) + (Units::winpct(WIDTH) / 3);
        optionWidth = Units::winpct(WIDTH) / 3;
        optionRight = optionLeft + optionWidth;
    }

    float getEntryTop(int entryNum) const {
        return entryTop + entryHeight * (entryNum - 1);
    }

    float getEntryTextTop(int entryNum) const {
        return entryTextTop + entryHeight * (entryNum - 1);
    }
};

float settingsOptionRenderer::sliderEntry(
    float value, float min, float max, int entryNum, const std::string& label, float increment
) {
    static Layout layout;

    float lengthTop = layout.getEntryTop(entryNum);
    float lengthTextTop = layout.getEntryTextTop(entryNum);
    float* valuePtr = &value;

    DrawTextEx(
        ASSET(rubikBold),
        label.c_str(),
        { layout.entryTextLeft, lengthTextTop },
        layout.entryFontSize,
        0,
        WHITE
    );

    if (GuiSliderBar(
            { layout.optionLeft + layout.entryHeight,
              lengthTop,
              layout.optionWidth - (layout.entryHeight * 2),
              layout.entryHeight },
            "",
            "",
            valuePtr,
            min,
            max
        )) {
        value = *valuePtr;
    }

    if (GuiButton({ layout.optionLeft, lengthTop, layout.entryHeight, layout.entryHeight }, "<")) {
        value -= increment;
    }
    if (GuiButton({ layout.optionRight - layout.entryHeight, lengthTop, layout.entryHeight, layout.entryHeight }, ">")) {
        value += increment;
    }

    std::string valueStr = trFloatString(value);
    float valueMiddle = MeasureTextEx(ASSET(rubikBold), valueStr.c_str(), layout.entryFontSize, 0).x / 2;
    DrawTextEx(
        ASSET(rubikBold),
        valueStr.c_str(),
        { layout.optionRight - (layout.optionWidth / 2) - valueMiddle, lengthTextTop },
        layout.entryFontSize,
        0,
        WHITE
    );


    return value;
}

bool settingsOptionRenderer::toggleEntry(bool value, int entryNum, const std::string& label) {
    static Layout layout;

    float valueTop = layout.getEntryTop(entryNum);
    float valueTextTop = layout.getEntryTextTop(entryNum);

    DrawTextEx(
        ASSET(rubikBold),
        label.c_str(),
        { layout.entryTextLeft, valueTextTop },
        layout.entryFontSize,
        0,
        WHITE
    );

    if (GuiButton({ layout.optionLeft, valueTop, layout.optionWidth, layout.entryHeight }, value ? "On" : "Off")) {
        value = !value;
    }
    return value;
}

static void renderKeybindButton(
    float left, float top, float width, float height,
    const std::string& label, const std::string& keyStr,
    std::function<void()> onClick
) {
    DrawTextEx(ASSET(rubikBold), label.c_str(), { left - 100, top + 5 }, 20, 0, WHITE);
    if (GuiButton({ left, top, width, height }, keyStr.c_str())) {
        onClick();
    }
}

void settingsOptionRenderer::keybindEntryText(int entryNum, const std::string& label) {
    static Layout layout;
    float valueTextTop = layout.getEntryTextTop(entryNum);

    DrawTextEx(
        ASSET(rubikBold),
        label.c_str(),
        { layout.entryTextLeft, valueTextTop },
        layout.entryFontSize,
        0,
        WHITE
    );
}