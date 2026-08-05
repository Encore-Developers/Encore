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

std::string XBtempLaneToButtonLabel(Encore::InputChannel channel) {
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

std::string NStempLaneToButtonLabel(Encore::InputChannel channel) {
    switch (channel) {
    case Encore::InputChannel::LANE_1: return "B";
    case Encore::InputChannel::LANE_2: return "A";
    case Encore::InputChannel::LANE_3: return "X";
    case Encore::InputChannel::LANE_4: return "Y";
    case Encore::InputChannel::LANE_5: return "L";
    case Encore::InputChannel::STRUM_UP: return "^";
    case Encore::InputChannel::STRUM_DOWN: return "v";
    case Encore::InputChannel::PAUSE: return "+";
    case Encore::InputChannel::OVERDRIVE: return "-";
    case Encore::InputChannel::INPUT_LEFT: return "<";
    case Encore::InputChannel::INPUT_RIGHT: return ">";
    default: return "";
    }
}

std::string PStempLaneToButtonLabel(Encore::InputChannel channel) {
    switch (channel) {
    case Encore::InputChannel::LANE_1: return "X";
    case Encore::InputChannel::LANE_2: return "O";
    case Encore::InputChannel::LANE_3: return "Triangle";
    case Encore::InputChannel::LANE_4: return "[]";
    case Encore::InputChannel::LANE_5: return "L1";
    case Encore::InputChannel::STRUM_UP: return "^";
    case Encore::InputChannel::STRUM_DOWN: return "v";
    case Encore::InputChannel::PAUSE: return "-";
    case Encore::InputChannel::OVERDRIVE: return "<";
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

Color PStempColorToButtonLabel(Encore::InputChannel channel) {
    switch (channel) {
    case Encore::InputChannel::LANE_1: return SKYBLUE;
    case Encore::InputChannel::LANE_2: return RED;
    case Encore::InputChannel::LANE_3: return GREEN;
    case Encore::InputChannel::LANE_4: return VIOLET;
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

enum ConType : size_t {
    cGUITAR = 0,
    cPS,
    cNS,
    cXB
};

void Encore::ButtonActionRegistry::DrawPrompts(bool OvershellOpen, float top, float left) {
    ZoneScoped
    std::array<bool, 4> types = {};
    for (auto& player : ThePlayerManager.ActivePlayers) {
        if (player == nullptr) continue;
        auto type = SDL_GetJoystickType(SDL_GetJoystickFromID(player->joypadID));
        char guidStr[33];
        SDL_GUIDToString(SDL_GetJoystickGUIDForID(player->joypadID), guidStr, 33);
        std::string guid = guidStr;
        if (OvershellMenu::hardcodedControllerTypes.contains(guid)) {
            types.at(cGUITAR) = true;;
            return;
        }
        switch (type) {
        case SDL_JOYSTICK_TYPE_GUITAR:
        case SDL_JOYSTICK_TYPE_DRUM_KIT:
            types.at(cGUITAR) = true;
            break;
        case SDL_JOYSTICK_TYPE_GAMEPAD: {
            switch (SDL_GetGamepadTypeForID(player->joypadID)) {
            case SDL_GAMEPAD_TYPE_STANDARD:
            case SDL_GAMEPAD_TYPE_XBOX360:
            case SDL_GAMEPAD_TYPE_XBOXONE:
            case SDL_GAMEPAD_TYPE_GAMECUBE:
            case SDL_GAMEPAD_TYPE_UNKNOWN:
            case SDL_GAMEPAD_TYPE_STEAM:
                types.at(cXB) = true;
                break;
            case SDL_GAMEPAD_TYPE_PS3:
            case SDL_GAMEPAD_TYPE_PS4:
            case SDL_GAMEPAD_TYPE_PS5:
                types.at(cPS) = true;
                break;
            case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
            case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
            case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
            case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
                types.at(cNS) = true;
                break;
            default: types.at(cXB) = true;
            }
        }
        }
    }
    int totalTypes = 0;
    for (int a = 1; a < 4 ; a++) {
        totalTypes += types.at(a);
    }
    Units u = Units::getInstance();
    bool modified = false;
    if (top < 0) {
        modified = true;
        top = GetRenderHeight() - u.hpct(0.18f);
    }
    if (left < 0) {
        modified = true;
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

    float buttonIconWidth = buttonHeight * 1.3f;
    float spacer = totalTypes + 1;
    float asdgasdfkjahlsdg = totalTypes > 1 ? (buttonIconWidth * 0.95f) + ((totalTypes-1) * (buttonHeight*0.6f)) : buttonIconWidth * 0.95f;

    Rectangle icon {
        pos.x + u.hinpct(0.005f),
        backgroundPos.y + (backgroundPos.height * 0.15f),
        asdgasdfkjahlsdg,
        backgroundPos.height * 0.7f
    };

    NPatchInfo iconNP;
    iconNP.left = (buttonIconWidth * 0.95f) / 3;
    iconNP.right = (buttonIconWidth * 0.95f) / 3;
    iconNP.source = {0,0,float(ASSET(fretButtonPrompt).width), float(ASSET(fretButtonPrompt).height)};
    iconNP.layout = NPATCH_THREE_PATCH_HORIZONTAL;

    ButtonData.Pos(left + (icon.width + (buttonHeight * 0.25)), top + u.hinpct(0.015f)).Size(nameFontSize).Fnt(ASSET(josefinSansBold));
    if (modified) {
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
        pos.width = textWidth + icon.width + (buttonHeight * 0.45);
        backgroundPos.width = textWidth + icon.width + (buttonHeight * 0.45);
        DrawRectangleRounded(backgroundPos, 0.5, 10, {0,0,0,64});
        bool IsHovered = CheckCollisionPointRec(GetMousePosition(), pos);
        bool IsClicked = IsHovered && IsMouseButtonPressed(0);
        if (!OvershellOpen && IsHovered) {
            DrawRectangleRounded(backgroundPos, 0.5, 10, {255,0,255,64});
        }
        if (!OvershellOpen && IsClicked) {
            butt.second.RunAction(Action::PRESS, -1);
        };



        Color buttColor = tempColorToButtonLabel(butt.first);
        DrawTextureNPatch(ASSET(fretButtonPrompt), iconNP, icon, {0}, 0, buttColor);

        if (types.at(cXB)) {
            icon.x += (asdgasdfkjahlsdg/spacer);
            DrawCircle(icon.x, icon.y + (icon.height/2), icon.height * 0.5, {BLACK.r, BLACK.g, BLACK.b, 196});

            Text::DrawText(ASSET(rubikBold),
                                 XBtempLaneToButtonLabel(butt.first),
                                 { icon.x,
                                   icon.y + ((icon.height/2) - ((fontSize * 0.65f)/2)) },
                                 fontSize * 0.65f,
                                 tempColorToButtonLabel(butt.first),
                                 CENTER);

        }
        if (types.at(cPS)) {
            icon.x += (asdgasdfkjahlsdg/spacer);
            DrawCircle(icon.x, icon.y + (icon.height/2), icon.height * 0.5, {BLACK.r, BLACK.g, BLACK.b, 196});

            Text::DrawText(ASSET(rubikBold),
                                 PStempLaneToButtonLabel(butt.first),
                                 { icon.x,
                                   icon.y + ((icon.height/2) - ((fontSize * 0.65f)/2)) },
                                 fontSize * 0.65f,
                                 PStempColorToButtonLabel(butt.first),
                                 CENTER);
        }
        if (types.at(cNS)) {
            icon.x += (asdgasdfkjahlsdg/spacer);
            DrawCircle(icon.x, icon.y + (icon.height/2), icon.height * 0.5, {BLACK.r, BLACK.g, BLACK.b, 196});

            Text::DrawText(ASSET(rubikBold),
                                 NStempLaneToButtonLabel(butt.first),
                                 { icon.x,
                                   icon.y + ((icon.height/2) - ((fontSize * 0.65f)/2)) },
                                 fontSize * 0.65f,
                                 RAYWHITE,
                                 CENTER);
        }
        // todo: replace this with actual controller-dependant icons


        ButtonData.lDrawText(butt.second.Name);
        ButtonData.pos.x += pos.width + (buttonHeight * 0.25);
        pos.x += pos.width + (buttonHeight * 0.25);
        icon.x = pos.x + u.hinpct(0.005f);
        backgroundPos.x += pos.width + (buttonHeight * 0.25);
    }
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
}