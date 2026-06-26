//
// Created by maria on 21/05/2026.
//

#include "ButtonActionRegistry.h"

#include "raygui.h"
#include "menus/util/locale/Locale.h"
#include "menus/MenuManager.h"
#include "uiUnits.h"
#include "menus/main/MainMenu.h"

// these funcs below are temp before a proper binding/icon library is implemented

std::string tempLaneToButtonLabel(Encore::InputChannel channel) {
    switch (channel) {
    case Encore::InputChannel::LANE_1: return "A";
    case Encore::InputChannel::LANE_2: return "B";
    case Encore::InputChannel::LANE_3: return "Y";
    case Encore::InputChannel::LANE_4: return "X";
    case Encore::InputChannel::LANE_5: return "LB";
    case Encore::InputChannel::STRUM_UP: return "^";
    case Encore::InputChannel::STRUM_DOWN: return "v";
    case Encore::InputChannel::PAUSE: return "+";
    case Encore::InputChannel::OVERDRIVE: return "-";
    case Encore::InputChannel::INPUT_LEFT: return "<";
    case Encore::InputChannel::INPUT_RIGHT: return ">";
    default: return "";
    }
}

Color tempColorToButtonLabel(Encore::InputChannel channel) {
    switch (channel) {
    case Encore::InputChannel::LANE_1: return GREEN;
    case Encore::InputChannel::LANE_2: return RED;
    case Encore::InputChannel::LANE_3: return YELLOW;
    case Encore::InputChannel::LANE_4: return BLUE;
    case Encore::InputChannel::LANE_5: return ORANGE;
    default: return LIGHTGRAY;
    }
}

void Encore::ButtonActionRegistry::HandleInput(const ControllerEvent &event) {
    int curSlot = 0;
    if (ThePlayerManager.GetPlayerForJoystick(event.slot)) {
        curSlot = ThePlayerManager.GetPlayerForJoystick(event.slot)->ActiveSlot;
    }
    if (buttMap.contains(event.channel)) {
        Log::Trace("Button Action {} (channel: {}) pressed by slot {}", LOCALISE(buttMap.at(event.channel).Name).toString(), ICInt(event.channel), curSlot);
        buttMap.at(event.channel).RunAction(event.action, curSlot);
    }
}

void Encore::ButtonActionRegistry::DrawPrompts(bool OvershellOpen, float top, float left) {
    ZoneScoped
    Units u = Units::getInstance();
    if (top < 0) {
        top = GetRenderHeight() - u.hpct(0.18f);
    }
    if (left < 0) {
        left = u.LeftSide;
    }
    GuiSetStyle(BUTTON,
                BASE_COLOR_NORMAL,
                0x00000000);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
    float ButtonWidth = u.winpct(0.135f);
    const float buttonHeight = u.hinpct(0.05f);
    const float fontSize = u.hinpct(0.03f);
    const float nameFontSize = u.hinpct(0.025f);
    Rectangle pos = {left, top, ButtonWidth, buttonHeight};
    Rectangle backgroundPos = {left, top + u.hinpct(0.0075f), ButtonWidth - u.hinpct(0.0075f), buttonHeight - u.hinpct(0.015f)};
    TextDisplay ButtonData;
    ButtonData.Pos(left + buttonHeight, top + u.hinpct(0.015f)).Size(nameFontSize).Fnt(ASSET(josefinSansBold));
    {
        float BottomOvershell = GetRenderHeight() - u.hpct(0.18f);
        DrawRectangleGradientV(
            0,
            BottomOvershell,
            (float)(GetRenderWidth()),
            u.hinpct(0.05f),
        GetColor(0x472E47FF),
        GetColor(0x271827FF)
        );
    }
    for (auto &butt : buttMap) {
        if (!butt.second.barVisible)
            continue;
        float textWidth = ButtonData.lTextWidth(butt.second.Name);
        pos.width = textWidth + (buttonHeight * 1.5);
        backgroundPos.width = textWidth + (buttonHeight * 1.25);
        DrawRectangleRounded(backgroundPos, 0.5, 10, {0,0,0,64});
        bool IsHovered = CheckCollisionPointRec(GetMousePosition(), pos);
        bool IsClicked = IsHovered && IsMouseButtonPressed(0);
        if (!OvershellOpen && IsHovered) {
            DrawRectangleRounded(backgroundPos, 0.5, 10, {255,0,255,64});
        }
        if (!OvershellOpen && IsClicked) {
            butt.second.RunAction(Action::PRESS, -1);
        };

        // todo: replace this with actual controller-dependant icons
        Text::DrawText(ASSET(rubikBold),
                             tempLaneToButtonLabel(butt.first),
                             { pos.x + u.hinpct(0.01f),
                               pos.y + u.hinpct(0.01f) },
                             fontSize,
                             tempColorToButtonLabel(butt.first),
                             LEFT);

        ButtonData.lDrawText(butt.second.Name);
        ButtonData.pos.x += pos.width;
        pos.x += pos.width;
        backgroundPos.x += pos.width;
    }
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
}