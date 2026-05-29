//
// Created by maria on 23/05/2026.
//

#include "SettingRenderer.h"

#include "raygui.h"
#include "menus/uiUnits.h"
#include "menus/main/MainMenu.h"

void Encore::SettingDoohickey::Action(bool remove) {
    if (settingsArray.at(selectedIndex))
        settingsArray.at(selectedIndex)->Action(remove);
}

void Encore::SettingDoohickey::IncrementSelected(bool up) {
    int val = up ? -1 : 1;
    selectedIndex += val;

    if (selectedIndex < 0) selectedIndex = 0;
    if (selectedIndex >= settingsArray.size()) selectedIndex = settingsArray.size()-1;
    if (settingsArray.at(selectedIndex)->GetType() == settingType::SEPARATOR) {
        if (selectedIndex == 0) {
            selectedIndex = 1;
        } else {
            selectedIndex += val;
        }
    }
}

void Encore::SettingDoohickey::Draw(float EntryTop) {
    Units u = Units::getInstance();
    float EntryHeight = u.hinpct(0.05f);
    float EntryWidth = u.winpct(0.7);

    float EntryLeft = ((u.winpct(1.0) - EntryWidth) / 2) + u.LeftSide;
    Vector2 mousePos = GetMousePosition();

    Color sliderNormal = Color{24, 24, 39, 178};
    Color sliderHovered = Color{84, 13, 88, 200};
    Color sliderFocused = Color{142, 13, 148, 220};

    GuiSetStyle(SLIDER, BASE_COLOR_NORMAL, ColorToInt(sliderNormal));
    GuiSetStyle(SLIDER, BASE_COLOR_FOCUSED, ColorToInt(sliderHovered));
    GuiSetStyle(SLIDER, BASE_COLOR_PRESSED, ColorToInt(sliderFocused));
    GuiSetStyle(SLIDER, TEXT_COLOR_FOCUSED, ColorToInt(sliderFocused));
    GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(SLIDER, BORDER_WIDTH, 2);
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 0);
    GuiSetStyle(SLIDER, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    Rectangle background { EntryLeft, u.hpct(EntryTop), EntryWidth, GetRenderHeight() - u.hpct(0.15f)};
    DrawTexturePro(ASSET(EntryBackground), {0, 0, (float)ASSET(EntryBackground).width, (float)ASSET(EntryBackground).height}, background, {0}, 0, { 255, 255, 255, 128});

    for (int settingOffset = 0; settingOffset < settingsArray.size(); settingOffset++) {
        float top = u.hpct(EntryTop) + (EntryHeight * settingOffset);
        Rectangle wholeBoxRect = {EntryLeft, top, EntryWidth, EntryHeight};
        bool hovered = false;
        if (CheckCollisionPointRec(mousePos, wholeBoxRect)) {
            selectedIndex = settingOffset;
        }
        if (selectedIndex == settingOffset) {
            hovered = true;
        }
        settingsArray.at(settingOffset)->Draw(wholeBoxRect, hovered, isOSOpen);

    }
}