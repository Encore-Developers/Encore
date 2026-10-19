#include "OvershellHelper.h"
#include "../util/uiUnits.h"
#include "raygui.h"
#include "assets.h"
#include "../util/styles.h"
#include "menus/util/TextDisplay.h"

void encOS::DrawBeacon(int slot, float x, float y, float width, float height, bool top, Color playerColor) {
    Color overshellBeacon = ColorBrightness(playerColor, -0.75f);
    Color thanksraylib = { overshellBeacon.r, overshellBeacon.g, overshellBeacon.b, 128 };
    float HalfWidth = width / 2;
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
    for (int g = 0; g < 4; g++) {
        DrawRectangleGradientH(x, y, HalfWidth, height, { 0, 0, 0, 0 }, thanksraylib);
        DrawRectangleGradientH(
            x + HalfWidth - 1, y, HalfWidth, height, thanksraylib, { 0, 0, 0, 0 }
        );
    }
    EndBlendMode();
    Color BaseWitAllAlpha = ColorBrightness(GetColor(0x181827FF), -0.25f);
    Color BaseWitNoAlpha = { BaseWitAllAlpha.r, BaseWitAllAlpha.g, BaseWitAllAlpha.b, 0 };
    if (top) {
        DrawRectangleGradientV(x, y, width, height, BaseWitAllAlpha, BaseWitNoAlpha);
    }
}

// TODO remove these damn parameters we don't need y, width, or height
bool encOS::DrawOvershellRectangleHeader(
    float x,
    float y,
    float width,
    float height,
    std::string username,
    Color accentColor,
    Color usernameColor,
    bool drawBG
) {
    height = Units::hinpct(0.11f);
    y = GetYPos(1) - height;
    Assets &assets = Assets::getInstance();
    float UpperPortion = height * 0.6f;
    float LowerPortion = height * 0.4f;
    Rectangle RectPos = { x, y, width, height };
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0);
    GuiSetStyle(BUTTON, BACKGROUND_COLOR, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0);
    bool toReturn = GuiButton({ x, y, width, height }, "");
    SETDEFAULTSTYLE();
    float patchWidth = (UpperPortion / 3) * 2;
    float asdfasdfas = width - (patchWidth * 2);
    DrawTexturePro(ASSET(overshellTip), {0,0,85,128}, {x,y,patchWidth,UpperPortion}, {0}, 0, accentColor);
    DrawTexturePro(ASSET(overshellTip), {85,0,85,128}, {x+patchWidth,y,asdfasdfas,UpperPortion}, {0}, 0, accentColor);
    DrawTexturePro(ASSET(overshellTip), {255-85,0,85,128}, {x+asdfasdfas+patchWidth,y,patchWidth,UpperPortion}, {0}, 0, accentColor);
    // float Inset = unit.winpct(0.001f);
    // float InsetDouble = Inset * 2;
    // DrawRectangleRounded(
    //     {RectPos.x + Inset, RectPos.y + Inset, RectPos.width - (InsetDouble*1.25f),
    //     RectPos.height - InsetDouble}, 0.40f, 5, ColorBrightness(accentColor, -0.75f)
    //);
    //int fgasdf = UpperPortion;
    //int g = y;
    //BeginScissorMode(x, g, width + 2, fgasdf);
    //DrawRectangleRounded(RectPos, 0.25f, 8, ColorBrightness(accentColor, -0.5f));
    //EndScissorMode();
    if (drawBG) {
        DrawRectangleRec({x, y+UpperPortion, width, GetRenderHeight()-y}, {0x18, 0x18, 0x27, 0xFF});
    }

    float centerPos = x + (width / 2);
    float fontSize = UpperPortion / 1.75;
    float padding = Units::hinpct(0.025f);
    float textX = x + padding;
    float textY = y + ((UpperPortion/2) - (fontSize/2));
    float textW = width - (padding*2);
    float textH = UpperPortion - (padding*2);
    Encore::TextDisplay name;
    name.Pos(textX,textY).Bounds(textW, textH).Align(CENTER).Col(usernameColor).Fnt(ASSET(redHatDisplayBlack)).Size(fontSize).DrawText(username);
    return toReturn;
}

bool encOS::DrawOvershellBottomCover(float x,
    float width,
    int state,
    Color accentColor) {
    float height = Units::hinpct(0.11f * 0.4f);
    DrawRectangleGradientV(x, GetYPos(0, true), width, height, ColorBrightness(accentColor, -0.6f), ColorBrightness(accentColor, -0.76f));
    DrawRectangleGradientV(x, GetYPos(0, true), width, Units::hinpct(0.004f), Color { 0, 0, 0, 64 }, Color { 0, 0, 0, 0 });
    return true;
}

float encOS::GetYPos(int buttonIndex, bool ignoreRaise) {
    float baseline = Units::hpct(1.0) - Units::hinpct(0.11f * 0.4f);
    auto out = baseline + (Units::winpct(0.03f) * (buttonIndex));
    if (OvershellInputState::currentState && !ignoreRaise) {
        out -= OvershellInputState::currentState->raise * Units::winpct(0.03f);
    }
    return out;
}

bool encOS::OvershellButton(int slot, int x, std::string string) {
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0);
    float y = Units::hpct(1.0f) - Units::hinpct(0.11f * 0.4f);
    bool selected = GuiButton(
        { osLeft,
          GetYPos(x),
          osWidth,
          Units::winpct(0.03f) },
        string.c_str()
    );
    SETDEFAULTSTYLE();
    if (OvershellInputState::currentState && OvershellInputState::currentState->focusedItem == x) {
        DrawRectangle(osLeft, GetYPos(x), osWidth, Units::winpct(0.03f), {255, 0, 255, 80});
        if (OvershellInputState::currentState->selectPressed) {
            selected = true;
        }
    }
    return selected;
}

void encOS::OvershellText(int slot, int x, std::string string) {
    SETDEFAULTSTYLE();
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);
    GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD);
    GuiSetStyle(DEFAULT, TEXT_LINE_SPACING, Units::hinpct(0.03f));
    float y = Units::hpct(1.0f) - Units::hinpct(0.11f * 0.4f);
    GuiLabel(
        { osLeft,
          y - (Units::winpct(0.03f) * (x + 1)),
          osWidth,
          Units::winpct(0.07f) },
        string.c_str()
    );
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_CENTER);
    GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_NONE);
}

bool encOS::OvershellCheckbox(int slot, int x, std::string string, bool initialVal) {
    float height = Units::winpct(0.03f);
    float widthNoHeight = Units::winpct(0.2f);
    float y = Units::hpct(1.0f) - Units::hinpct(0.11f * 0.4f);
    Rectangle bounds = { osLeft + height,
                         Units::hpct(1.0f) - (Units::winpct(0.03f) * (x + 1)),
                         Units::winpct(0.2f) - height - height,
                         height };
    Rectangle confirmBounds = { osLeft + osWidth - height,
                                GetYPos(x),
                                height,
                                height };
    Assets &assets = Assets::getInstance();

    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0);

    if (GuiButton(
            { osLeft,
              GetYPos(x),
              osWidth,
              height },
            string.c_str()
        )) {
        initialVal = !initialVal;
    }

    SETDEFAULTSTYLE();

    if (OvershellInputState::currentState && OvershellInputState::currentState->focusedItem == x) {
        DrawRectangle(osLeft, GetYPos(x), osWidth, Units::winpct(0.03f), {255, 0, 255, 80});
        if (OvershellInputState::currentState->selectPressed) {
            initialVal = !initialVal;
        }
    }

    DrawRectanglePro(confirmBounds, { 0 }, 0, initialVal ? GREEN : RED);
    return initialVal;
}


bool encOS::OvershellSlider(
    int slot, int x, std::string string, float *value, float step, float min, float max
) {
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0);
    float height = Units::winpct(0.03f);
    float speedTextWidth = Units::winpct(0.04f);
    float widthNoHeight = Units::winpct(0.2f) - height;
    Rectangle bounds = { osLeft + speedTextWidth,
                         GetYPos(x),
                         osWidth - speedTextWidth - height,
                         height };
    Rectangle confirmBounds = { osLeft + osWidth - height,
                                GetYPos(x),
                                height,
                                height };
    Assets &assets = Assets::getInstance();

    GuiSlider(bounds, "", "", value, min, max);
    GuiButton(
        { osLeft,
          GetYPos(x),
          speedTextWidth,
          height },
        TextFormat("%1.2f", *value)
    );
    *value = (round(*value / step) * step);

    SETDEFAULTSTYLE();

    if (OvershellInputState::currentState) {
        OvershellInputState::currentState->blockNav = true;
        if (OvershellInputState::currentState->upPressed) {
            *value += step;
        }
        if (OvershellInputState::currentState->downPressed) {
            *value -= step;
        }
        if (OvershellInputState::currentState->selectPressed || OvershellInputState::currentState->backPressed) {
            return true;
        }
    }

    if (GuiButton(confirmBounds, "<")) {
        return true;
    };
    return false;
}
