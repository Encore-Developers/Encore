#include "SettingRenderer.h"
#include "raygui.h"
#include "menus/uiUnits.h"
#include "menus/locale/Locale.h"
#include "menus/main/MainMenu.h"

Color glowColor = Color{142, 13, 148, 64};
Color activeColor = Color{255, 105, 180, 255};

void Encore::SettingDoohickey::boolSettingObject::Draw(Rectangle pos, bool hovered, bool Clickable) {
    Units u = Units::getInstance();
    float EntryFontSize = u.hinpct(0.03f);
    int defaultColor = GuiGetStyle(BUTTON, BASE_COLOR_PRESSED);
    float TextTop = pos.y + (pos.height - EntryFontSize) / 2;

    DrawTexturePro(ASSET(EntryBackground), {0, 0, (float)ASSET(EntryBackground).width, (float)ASSET(EntryBackground).height}, pos, {0}, 0, WHITE);

    Rectangle ButtonSpace = {pos.x + ((pos.width / 3) * 2), pos.y, pos.width / 3, pos.height};
    if (hovered) {
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangleRec(pos, glowColor);
        EndBlendMode();
    }
    GameMenu::mhDrawText(ASSET(rubikBold), LOCALISE(name), {pos.x + u.winpct(0.01f), TextTop}, EntryFontSize, WHITE, ASSET(sdfShader), LEFT);
    Rectangle offRect = {ButtonSpace.x, ButtonSpace.y, ButtonSpace.width/2, ButtonSpace.height};
    Rectangle onRect = {ButtonSpace.x + ButtonSpace.width/2, ButtonSpace.y, ButtonSpace.width/2, ButtonSpace.height};

    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, *value ? defaultColor : ColorToInt(activeColor));
    if (GuiButton(offRect, "") && !Clickable) {
        *value = false;
    }
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, *value ? ColorToInt(activeColor) : defaultColor);
    if (GuiButton(onRect, "") && !Clickable) {
        *value = true;
    }
    GameMenu::mhDrawText(ASSET(rubik), LOCALISE("generic.on"), {onRect.x + onRect.width/2, TextTop}, EntryFontSize, LIGHTGRAY, ASSET(sdfShader), CENTER);
    GameMenu::mhDrawText(ASSET(rubik), LOCALISE("generic.off"), {offRect.x + offRect.width/2, TextTop}, EntryFontSize, LIGHTGRAY, ASSET(sdfShader), CENTER);

    if (!*value) {
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangleRec(offRect, {glowColor.r, glowColor.g, glowColor.b, (unsigned char)(glowColor.a*2)});
        EndBlendMode();
    } else {
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangleRec(onRect, {glowColor.r, glowColor.g, glowColor.b, (unsigned char)(glowColor.a*2)});
        EndBlendMode();
    }
}

void Encore::SettingDoohickey::floatSettingObject::Draw(Rectangle pos, bool hovered, bool Clickable) {
    Units u = Units::getInstance();
    float EntryFontSize = u.hinpct(0.03f);
    float TextTop = pos.y + (pos.height - EntryFontSize) / 2;
    DrawTexturePro(ASSET(EntryBackground), {0, 0, (float)ASSET(EntryBackground).width, (float)ASSET(EntryBackground).height}, pos, {0}, 0, WHITE);

    Rectangle ButtonSpace = {pos.x + ((pos.width / 3) * 2), pos.y, pos.width / 3, pos.height};
    if (hovered) {
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangleRec(pos, glowColor);
        EndBlendMode();
    }
    GameMenu::mhDrawText(ASSET(rubikBold), LOCALISE(name), {pos.x + u.winpct(0.01f), TextTop}, EntryFontSize, WHITE, ASSET(sdfShader), LEFT);
    ::GuiSlider(ButtonSpace, nullptr, nullptr, value, min, max);
    GameMenu::mhDrawText(ASSET(rubik), TextFormat("%i%%", int(*value * 100)), {ButtonSpace.x + (ButtonSpace.width / 2), TextTop}, EntryFontSize, LIGHTGRAY, ASSET(sdfShader), CENTER);
}

void Encore::SettingDoohickey::buttonSettingObject::Draw(Rectangle pos, bool hovered, bool Clickable) {
    Units u = Units::getInstance();
    float EntryFontSize = u.hinpct(0.03f);
    float TextTop = pos.y + (pos.height - EntryFontSize) / 2;
    DrawTexturePro(ASSET(EntryBackground), {0, 0, (float)ASSET(EntryBackground).width, (float)ASSET(EntryBackground).height}, pos, {0}, 0, WHITE);


    if (hovered) {
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangleRec(pos, glowColor);
        EndBlendMode();
    }

    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, pos) && !Clickable) {
        value->operator()();
    }
    GameMenu::mhDrawText(ASSET(rubikBold), LOCALISE(name), {pos.x + (pos.width /2), TextTop}, EntryFontSize, WHITE, ASSET(sdfShader), CENTER);
}

void Encore::SettingDoohickey::separatorObject::Draw(Rectangle pos, bool hovered, bool Clickable) {
    Units u = Units::getInstance();
    float EntryFontSize = u.hinpct(0.03f);
    float TextTop = pos.y + ((pos.height - EntryFontSize) / 3) * 2;
    GameMenu::mhDrawText(ASSET(rubikBold), LOCALISE(name), {pos.x + u.winpct(0.01f), TextTop}, EntryFontSize, WHITE, ASSET(sdfShader), LEFT);
    DrawLineEx({pos.x, pos.y + pos.height-2}, {pos.x + pos.width, pos.y + pos.height-2}, 2, WHITE);
}

void Encore::SettingDoohickey::intSettingObject::Draw(Rectangle pos, bool hovered, bool Clickable) {
    Units u = Units::getInstance();
    float EntryFontSize = u.hinpct(0.03f);
    float TextTop = pos.y + (pos.height - EntryFontSize) / 2;
    DrawTexturePro(ASSET(EntryBackground), {0, 0, (float)ASSET(EntryBackground).width, (float)ASSET(EntryBackground).height}, pos, {0}, 0, WHITE);

    Rectangle ButtonSpace = {pos.x + ((pos.width / 3) * 2), pos.y, pos.width / 3, pos.height};
    if (hovered) {
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangleRec(pos, glowColor);
        EndBlendMode();
    }
    GameMenu::mhDrawText(ASSET(rubikBold), LOCALISE(name), {pos.x + u.winpct(0.01f), TextTop}, EntryFontSize, WHITE, ASSET(sdfShader), LEFT);
    Rectangle leftButton {ButtonSpace.x, ButtonSpace.y, ButtonSpace.height, ButtonSpace.height};
    Rectangle slider {ButtonSpace.x + leftButton.width, ButtonSpace.y, ButtonSpace.width - (leftButton.width*2), leftButton.width};
    Rectangle rightButton {ButtonSpace.x + ButtonSpace.width - leftButton.width, ButtonSpace.y, leftButton.width, leftButton.width};
    if (GuiButton(leftButton, TextFormat("-%i", increment)) && !Clickable) {
        *value -= increment;
        if (*value < min) *value = min;
    }
    float temp = static_cast<float>(*value);
    GuiSlider(slider, nullptr, nullptr, &temp, min,  max);
    if (!Clickable) {
        *value = static_cast<int>(roundf(temp));
        if (*value < min) *value = min;
        if (*value > max) *value = max;
    }
    if (GuiButton(rightButton, TextFormat("+%i", increment)) && !Clickable) {
        *value += increment;
        if (*value > max) *value = max;
    }
    GameMenu::mhDrawText(ASSET(rubikBold), TextFormat("%i", *value), {slider.x + (slider.width / 2), TextTop}, EntryFontSize, LIGHTGRAY, ASSET(sdfShader), CENTER);
}
void Encore::SettingDoohickey::boolSettingObject::Action(bool invert) {
    if (invert)
        *value = false;
    else
        *value = true;
}
void Encore::SettingDoohickey::floatSettingObject::Action(bool invert) {
    if (invert)
        *value -= increment;
    else
        *value += increment;
}
void Encore::SettingDoohickey::intSettingObject::Action(bool invert) {
    if (invert)
        *value -= increment;
    else
        *value += increment;
}

void Encore::SettingDoohickey::buttonSettingObject::Action(bool invert) {
    value->operator()();
}

void Encore::SettingDoohickey::separatorObject::Action(bool invert) {
}