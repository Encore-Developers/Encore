//
// Created by marie on 22/09/2024.
//

#ifndef FONTSIZEVALUES_H
#define FONTSIZEVALUES_H

constexpr float MenuHeader = 0.125f;
constexpr float LargeHeader = 0.05f;
constexpr float MediumHeader = 0.04f;
constexpr float SmallHeader = 0.035f;
constexpr float XLHeader = 0.06f;

constexpr int backgroundColorInt = 0x181827FF;
constexpr Color backgroundColor = {24,24,39,255};



#endif //FONTSIZEVALUES_H

#define SETDEFAULTSTYLE() \
GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);\
GuiSetStyle(\
    BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness({255,0,255,255}, -0.5))\
);\
GuiSetStyle(\
    BUTTON, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness({255,0,255,255}, -0.3))\
);\
GuiSetStyle(\
    SLIDER, BORDER_COLOR_FOCUSED, ColorToInt(ColorBrightness({255,0,255,255}, -0.3))\
);\
GuiSetStyle(\
    SLIDER, BORDER_COLOR_NORMAL, ColorToInt(ColorBrightness({255,0,255,255}, -0.5))\
);\
GuiSetStyle(SLIDER, BORDER_COLOR_PRESSED, ColorToInt({255,0,255,255}));\
GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);\
GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);\
GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);\
GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);\
GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xcbcbcbFF);\
GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);\
GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, 0xFFFFFFFF);\
GuiSetStyle(DEFAULT, TEXT_SIZE, 28);\
\
GuiSetStyle(TOGGLE, BASE_COLOR_NORMAL, 0x181827FF);\
GuiSetStyle(\
    TOGGLE, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness({255,0,255,255}, -0.5))\
);\
GuiSetStyle(\
    TOGGLE, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness({255,0,255,255}, -0.3))\
);\
GuiSetStyle(TOGGLE, BORDER_COLOR_NORMAL, 0xFFFFFFFF);\
GuiSetStyle(TOGGLE, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);\
GuiSetStyle(TOGGLE, BORDER_COLOR_PRESSED, 0xFFFFFFFF);\
GuiSetStyle(TOGGLE, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);\
GuiSetStyle(TOGGLE, TEXT_COLOR_PRESSED, 0xFFFFFFFF);
