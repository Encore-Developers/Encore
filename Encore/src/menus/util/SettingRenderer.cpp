//
// Created by maria on 23/05/2026.
//

#include "SettingRenderer.h"

#include "raygui.h"
#include "menus/uiUnits.h"
#include "menus/main/MainMenu.h"

void Encore::SettingDoohickey::Action(bool remove) {
    switch (settingsArray.at(selectedIndex)->GetType()) {
    case settingType::BOOL_SETTING: {
        boolSettingObject* object = dynamic_cast<boolSettingObject *>(settingsArray.at(selectedIndex));
        if (remove)
            *object->value = false;
        else
            *object->value = true;

        break;
    }
    case settingType::FLOAT_SETTING: {
        floatSettingObject* object = dynamic_cast<floatSettingObject *>(settingsArray.at(selectedIndex));
        if (remove)
            *object->value -= object->increment;
        else
            *object->value += object->increment;
        break;
    }
    case settingType::INT_SETTING: {
        intSettingObject* object = dynamic_cast<intSettingObject *>(settingsArray.at(selectedIndex));
        if (remove)
            *object->value -= object->increment;
        else
            *object->value += object->increment;
        break;
    }
    }
}

void Encore::SettingDoohickey::Draw(float EntryTop) {
    Units u = Units::getInstance();
    float EntryFontSize = u.hinpct(0.03f);
    float EntryHeight = u.hinpct(0.05f);
    float EntryLeft = u.LeftSide;
    float EntryWidth = u.winpct(0.7);
    Vector2 mousePos = GetMousePosition();

    Color glowColor = Color{142, 13, 148, 64};
    Color activeColor = Color{255, 105, 180, 255};
    int defaultColor = GuiGetStyle(BUTTON, BASE_COLOR_PRESSED);
    for (int settingOffset = 0; settingOffset < settingsArray.size(); settingOffset++) {
        float top = u.hpct(EntryTop) + (EntryHeight * settingOffset);
        Rectangle wholeBoxRect = {EntryLeft, top, EntryWidth, EntryHeight};
        DrawTexturePro(ASSET(EntryBackground), {0, 0, (float)ASSET(EntryBackground).width, (float)ASSET(EntryBackground).height}, wholeBoxRect, {0}, 0, WHITE);

        Rectangle wholeButtonSpace = {EntryLeft + EntryWidth - (EntryWidth / 3), top, EntryWidth / 3, EntryHeight};
        if (CheckCollisionPointRec(mousePos, wholeBoxRect)) {
            selectedIndex = settingOffset;
        }
        if (selectedIndex == settingOffset) {
            BeginBlendMode(BLEND_ADDITIVE);
            DrawRectangleRec(wholeBoxRect, glowColor);
            EndBlendMode();
        }
        switch (settingsArray.at(settingOffset)->GetType()) {
        case settingType::BOOL_SETTING: {
            boolSettingObject* object = dynamic_cast<boolSettingObject *>(settingsArray.at(settingOffset));
            GameMenu::mhDrawText(ASSET(rubikBold), object->name, {EntryLeft + u.winpct(0.01f), top + (EntryHeight - EntryFontSize) / 2}, EntryFontSize, WHITE, ASSET(sdfShader), LEFT);
            GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, *object->value ? defaultColor : ColorToInt(activeColor));
            Rectangle offRect = {wholeButtonSpace.x, wholeButtonSpace.y, wholeButtonSpace.width/2, wholeButtonSpace.height};
            Rectangle onRect = {wholeButtonSpace.x + wholeButtonSpace.width/2, wholeButtonSpace.y, wholeButtonSpace.width/2, wholeButtonSpace.height};
            if (GuiButton(offRect, "Off")) {
                *object->value = false;
            }
            GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, *object->value ? ColorToInt(activeColor) : defaultColor);
            if (GuiButton(onRect, "")) {
                *object->value = true;
            }
            GameMenu::mhDrawText(ASSET(rubik), "On", {onRect.x + onRect.width/2, top + (EntryHeight - EntryFontSize) / 2}, EntryFontSize, LIGHTGRAY, ASSET(sdfShader), CENTER);

            GameMenu::mhDrawText(ASSET(rubik), "Off", {offRect.x + offRect.width/2, top + (EntryHeight - EntryFontSize) / 2}, EntryFontSize, LIGHTGRAY, ASSET(sdfShader), CENTER);

            if (!*object->value) {
                BeginBlendMode(BLEND_ADDITIVE);
                DrawRectangleRec(offRect, {glowColor.r, glowColor.g, glowColor.b, (unsigned char)(glowColor.a*2)});
                EndBlendMode();
            } else {
                BeginBlendMode(BLEND_ADDITIVE);
                DrawRectangleRec(onRect, {glowColor.r, glowColor.g, glowColor.b, (unsigned char)(glowColor.a*2)});
                EndBlendMode();
            }
            break;
        }
        case settingType::FLOAT_SETTING: {
            floatSettingObject* object = dynamic_cast<floatSettingObject *>(settingsArray.at(settingOffset));
            GameMenu::mhDrawText(ASSET(rubikBold), object->name, {EntryLeft + u.winpct(0.01f), top + (EntryHeight - EntryFontSize) / 2}, EntryFontSize, WHITE, ASSET(sdfShader), LEFT);
            ::GuiSlider(wholeButtonSpace, nullptr, nullptr, object->value, object->min, object->max);
            GameMenu::mhDrawText(ASSET(rubik), TextFormat("%i%%", int(*object->value * 100)), {wholeButtonSpace.x + (wholeButtonSpace.width / 2), top + (EntryHeight - EntryFontSize) / 2}, EntryFontSize, LIGHTGRAY, ASSET(sdfShader), CENTER);
            break;
        }
        case settingType::INT_SETTING: {
            intSettingObject* object = dynamic_cast<intSettingObject*>(settingsArray.at(settingOffset));
            GameMenu::mhDrawText(ASSET(rubikBold), object->name, {EntryLeft + u.winpct(0.01f), top + (EntryHeight - EntryFontSize) / 2}, EntryFontSize, WHITE, ASSET(sdfShader), LEFT);
            Rectangle leftButton {wholeButtonSpace.x, wholeButtonSpace.y, wholeButtonSpace.height, wholeButtonSpace.height};
            Rectangle slider {wholeButtonSpace.x + leftButton.width, wholeButtonSpace.y, wholeButtonSpace.width - (leftButton.width*2), leftButton.width};
            Rectangle rightButton {wholeButtonSpace.x + wholeButtonSpace.width - leftButton.width, wholeButtonSpace.y, leftButton.width, leftButton.width};
            if (GuiButton(leftButton, TextFormat("-%i", object->increment))) {
                object->value -= object->increment;
                if (*object->value < object->min) *object->value = object->min;
            }
            float temp = static_cast<float>(*object->value);
            ::GuiSlider(slider, nullptr, nullptr, &temp, object->min,  object->max);
            *object->value = static_cast<int>(roundf(temp));
            if (*object->value < object->min) *object->value = object->min;
            if (*object->value > object->max) *object->value = object->max;
            if (GuiButton(rightButton, TextFormat("+%i", object->increment))) {
                object->value += object->increment;
                if (*object->value > object->max) *object->value = object->max;
            }
            GameMenu::mhDrawText(ASSET(rubikBold), TextFormat("%i", *object->value), {slider.x + (slider.width / 2), top + (EntryHeight - EntryFontSize) / 2}, EntryFontSize, LIGHTGRAY, ASSET(sdfShader), CENTER);
            break;
        }
        case settingType::BUTTON_SETTING: {
            buttonSettingObject* object = dynamic_cast<buttonSettingObject*>(settingsArray.at(settingOffset));
            if (GuiButton(wholeBoxRect, "")) {
                // this feels wrong
                object->value->operator()();
            }
            GameMenu::mhDrawText(ASSET(rubikBold), object->name, {wholeBoxRect.x + (wholeBoxRect.width / 2), top + (EntryHeight - EntryFontSize) / 2}, EntryFontSize, WHITE, ASSET(sdfShader), CENTER);
            break;
        }
        case settingType::INVALID: {
            GameMenu::mhDrawText(ASSET(rubikBold), settingsArray.at(settingOffset)->name, {EntryLeft + u.winpct(0.01f), top + (EntryHeight - EntryFontSize) / 2}, EntryFontSize, WHITE, ASSET(sdfShader), LEFT);
        }
        }
    }
}