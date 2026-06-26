#include "OvershellHelper.h"
#include "../util/uiUnits.h"
#include "raygui.h"
#include "assets.h"
#include "../util/styles.h"
#include "../main/MainMenu.h"

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
    Assets &assets = Assets::getInstance();
    Units &u = Units::getInstance();
    float UpperPortion = height * 0.6f;
    float LowerPortion = height * 0.4f;
    Rectangle RectPos = { x, y, width, height * 2 };
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0);
    GuiSetStyle(BUTTON, BACKGROUND_COLOR, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0);
    bool toReturn = GuiButton({ x, y, width, UpperPortion }, "");
    SETDEFAULTSTYLE();
    // float Inset = unit.winpct(0.001f);
    // float InsetDouble = Inset * 2;
    // DrawRectangleRounded(
    //     {RectPos.x + Inset, RectPos.y + Inset, RectPos.width - (InsetDouble*1.25f),
    //     RectPos.height - InsetDouble}, 0.40f, 5, ColorBrightness(accentColor, -0.75f)
    //);
    int fgasdf = UpperPortion;
    int g = y;
    BeginScissorMode(x, g, width + 2, fgasdf);
    DrawRectangleRounded(RectPos, 0.25f, 8, ColorBrightness(accentColor, -0.5f));
    EndScissorMode();
    if (drawBG) {
        DrawRectangleRec({x, y+height, width, GetRenderHeight()-y}, {0x18, 0x18, 0x27, 0xFF});
    }

    float centerPos = x + (width / 2);
    float fontSize = UpperPortion / 1.75;
    float padding = u.hinpct(0.025f);
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
    Units &u = Units::getInstance();
    float height = u.hinpct(0.11f * 0.4f);

    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0);
    GuiSetStyle(BUTTON, BACKGROUND_COLOR, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0);
    bool toReturn = GuiButton({ x, GetYPos(), width, height }, "");
    DrawRectangleGradientV(x, GetYPos(), width, height, ColorBrightness(accentColor, -0.6f), ColorBrightness(accentColor, -0.76f));
    DrawRectangleGradientV(x, GetYPos(), width, u.hinpct(0.004f), Color { 0, 0, 0, 64 }, Color { 0, 0, 0, 0 });
    return toReturn;
}

float encOS::GetYPos() {
    Units &u = Units::getInstance();
    return u.hpct(1.0) - u.hinpct(0.11f * 0.4f);
}

bool encOS::OvershellButton(int slot, int x, std::string string) {
    Units &u = Units::getInstance();
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0);
    float y = u.hpct(1.0f) - u.hinpct(0.11f * 0.4f);
    bool selected = GuiButton(
        { osLeft,
          y - (u.winpct(0.03f) * (x + 1)),
          osWidth,
          u.winpct(0.03f) },
        string.c_str()
    );
    SETDEFAULTSTYLE();
    if (OvershellInputState::currentState && OvershellInputState::currentState->focusedItem == x) {
        DrawRectangle(osLeft, y - (u.winpct(0.03f) * (x + 1)), osWidth, u.winpct(0.03f), {255, 0, 255, 80});
        if (OvershellInputState::currentState->selectPressed) {
            selected = true;
        }
    }
    return selected;
}

void encOS::OvershellText(int slot, int x, std::string string) {
    Units &u = Units::getInstance();
    SETDEFAULTSTYLE();
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);
    GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD);
    GuiSetStyle(DEFAULT, TEXT_LINE_SPACING, u.hinpct(0.03f));
    float y = u.hpct(1.0f) - u.hinpct(0.11f * 0.4f);
    GuiLabel(
        { osLeft,
          y - (u.winpct(0.03f) * (x + 1)),
          osWidth,
          u.winpct(0.07f) },
        string.c_str()
    );
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_CENTER);
    GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_NONE);
}

bool encOS::OvershellCheckbox(int slot, int x, std::string string, bool initialVal) {
    Units &u = Units::getInstance();
    float height = u.winpct(0.03f);
    float widthNoHeight = u.winpct(0.2f);
    float y = u.hpct(1.0f) - u.hinpct(0.11f * 0.4f);
    Rectangle bounds = { osLeft + height,
                         u.hpct(1.0f) - (u.winpct(0.03f) * (x + 1)),
                         u.winpct(0.2f) - height - height,
                         height };
    Rectangle confirmBounds = { osLeft + osWidth - height,
                                y - (u.winpct(0.03f) * (x + 1)),
                                height,
                                height };
    Assets &assets = Assets::getInstance();

    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0);

    if (GuiButton(
            { osLeft,
              y - (u.winpct(0.03f) * (x + 1)),
              osWidth,
              height },
            string.c_str()
        )) {
        initialVal = !initialVal;
    }

    SETDEFAULTSTYLE();

    if (OvershellInputState::currentState && OvershellInputState::currentState->focusedItem == x) {
        DrawRectangle(osLeft, y - (u.winpct(0.03f) * (x + 1)), osWidth, u.winpct(0.03f), {255, 0, 255, 80});
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
    Units &u = Units::getInstance();
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0);
    float height = u.winpct(0.03f);
    float speedTextWidth = u.winpct(0.04f);
    float widthNoHeight = u.winpct(0.2f) - height;
    Rectangle bounds = { osLeft + speedTextWidth,
                         GetYPos() - (u.winpct(0.03f) * (x + 1)),
                         osWidth - speedTextWidth - height,
                         height };
    Rectangle confirmBounds = { osLeft + osWidth - height,
                                GetYPos() - (u.winpct(0.03f) * (x + 1)),
                                height,
                                height };
    Assets &assets = Assets::getInstance();

    GuiSlider(bounds, "", "", value, min, max);
    GuiButton(
        { osLeft,
          GetYPos() - (u.winpct(0.03f) * (x + 1)),
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
