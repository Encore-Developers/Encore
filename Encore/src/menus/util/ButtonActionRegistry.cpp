//
// Created by maria on 21/05/2026.
//

#include "ButtonActionRegistry.h"

#include "raygui.h"
#include "menus/locale/Locale.h"
#include "menus/MenuManager.h"
#include "menus/uiUnits.h"
#include "menus/main/MainMenu.h"

// these funcs below are temp before a proper binding/icon library is implemented

std::string tempLaneToButtonLabel(Encore::RhythmEngine::InputChannel channel) {
    switch (channel) {
    case Encore::RhythmEngine::InputChannel::LANE_1: return "A";
    case Encore::RhythmEngine::InputChannel::LANE_2: return "B";
    case Encore::RhythmEngine::InputChannel::LANE_3: return "Y";
    case Encore::RhythmEngine::InputChannel::LANE_4: return "X";
    case Encore::RhythmEngine::InputChannel::LANE_5: return "LB";
    case Encore::RhythmEngine::InputChannel::STRUM_UP: return "^";
    case Encore::RhythmEngine::InputChannel::STRUM_DOWN: return "v";
    case Encore::RhythmEngine::InputChannel::PAUSE: return "+";
    case Encore::RhythmEngine::InputChannel::OVERDRIVE: return "-";
    case Encore::RhythmEngine::InputChannel::INPUT_LEFT: return "<";
    case Encore::RhythmEngine::InputChannel::INPUT_RIGHT: return ">";
    default: return "";
    }
}

Color tempColorToButtonLabel(Encore::RhythmEngine::InputChannel channel) {
    switch (channel) {
    case Encore::RhythmEngine::InputChannel::LANE_1: return GREEN;
    case Encore::RhythmEngine::InputChannel::LANE_2: return RED;
    case Encore::RhythmEngine::InputChannel::LANE_3: return YELLOW;
    case Encore::RhythmEngine::InputChannel::LANE_4: return BLUE;
    case Encore::RhythmEngine::InputChannel::LANE_5: return ORANGE;
    default: return LIGHTGRAY;
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
    const float ButtonStart = left;
    const float ButtonTop = top;
    const float buttonHeight = u.hinpct(0.05f);
    const float fontSize = u.hinpct(0.03f);
    Rectangle pos = {ButtonStart, ButtonTop, ButtonWidth, buttonHeight};
    for (auto &butt : buttMap) {
        if (!butt.second.barVisible)
            continue;
        auto name = LOCALIZE(butt.second.Name);
        float textWidth = MeasureTextEx(ASSET(rubik), name, fontSize, 0).x;
        pos.width = textWidth + (buttonHeight * 1.5);
        if (GuiButton(pos,"") && !OvershellOpen) {
            butt.second.RunAction(RhythmEngine::Action::PRESS, 0);
        };

        // todo: replace this with actual controller-dependant icons
        GameMenu::mhDrawText(ASSET(rubikBold),
                             tempLaneToButtonLabel(butt.first),
                             { pos.x +  + u.hinpct(0.01f),
                               pos.y + u.hinpct(0.01f) },
                             fontSize,
                             tempColorToButtonLabel(butt.first),
                             ASSET(sdfShader),
                             LEFT);

        GameMenu::mhDrawText(ASSET(rubik),
                             name,
                             { pos.x + buttonHeight,
                               pos.y + u.hinpct(0.01f) },
                             fontSize,
                             WHITE,
                             ASSET(sdfShader),
                             LEFT);
        pos.x += pos.width;
    }
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
}