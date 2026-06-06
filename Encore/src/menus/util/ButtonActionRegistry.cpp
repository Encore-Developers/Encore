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
        buttMap.at(event.channel).RunAction(event.action, curSlot);
    }
}

void Encore::ButtonActionRegistry::DrawPrompts(bool OvershellOpen, float top, float left) {
    ZoneScoped
    Units u = Units::getInstance();
    if (top < 0) {
        top = GetRenderHeight() - u.hpct(0.1475f);
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
    Rectangle pos = {left, top, ButtonWidth, buttonHeight};
    TextDisplay ButtonData;
    ButtonData.Pos(left + buttonHeight, top + u.hinpct(0.01f)).Size(fontSize);

    for (auto &butt : buttMap) {
        if (!butt.second.barVisible)
            continue;
        float textWidth = ButtonData.lTextWidth(butt.second.Name);
        pos.width = textWidth + (buttonHeight * 1.5);
        if (!OvershellOpen && GuiButton(pos,"")) {
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
    }
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
}