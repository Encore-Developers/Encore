#include "OvershellHelper.h"
#include "uiUnits.h"
#include "raygui.h"
#include "assets.h"
#include "styles.h"
#include "gameMenu.h"
#include "users/playerManager.h"

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

void encOS::DrawTopOvershell(double height) {
    BeginBlendMode(BLEND_ALPHA);
    Units &unit = Units::getInstance();
    DrawRectangleGradientV(
        0,
        unit.hpct(height) - 2,
        GetRenderWidth(),
        unit.hinpct(0.025f),
        Color { 0, 0, 0, 128 },
        Color { 0, 0, 0, 0 }
    );
    DrawRectangle(0, 0, (int)GetRenderWidth(), unit.hpct(height), WHITE);
    DrawRectangle(
        0,
        0,
        (int)GetRenderWidth(),
        unit.hpct(height) - unit.hinpct(0.005f),
        ColorBrightness(GetColor(0x181827FF), -0.25f)
    );

    for (int i = 0; i < 4; i++) {
        float OvershellTopLoc = unit.hpct(1.0f) - unit.winpct(0.05f);
        float OvershellLeftLoc =
            (unit.wpct(0.125) + (unit.winpct(0.25) * i)) - unit.winpct(0.1);
        float OvershellCenterLoc = (unit.wpct(0.125) + (unit.winpct(0.25) * i));
        float HalfWidth = OvershellCenterLoc - OvershellLeftLoc;
        if (ThePlayerManager.ActivePlayers[i] != -1) {
            DrawBeacon(
                i,
                OvershellLeftLoc,
                0,
                HalfWidth * 2,
                unit.hpct(height) - unit.hinpct(0.005f),
                true,
                ThePlayerManager.GetActivePlayer(i).AccentColor
            );
        }
    }
}

bool encOS::DrawOvershellRectangleHeader(
    float x,
    float y,
    float width,
    float height,
    std::string username,
    Color accentColor,
    Color usernameColor
) {
    Assets &assets = Assets::getInstance();
    Units &unit = Units::getInstance();
    Rectangle RectPos = { x, y, width, height * 2 };
    bool toReturn = GuiButton({ x, y, width, height }, "");
    // float Inset = unit.winpct(0.001f);
    // float InsetDouble = Inset * 2;
    // DrawRectangleRounded(
    //     {RectPos.x + Inset, RectPos.y + Inset, RectPos.width - (InsetDouble*1.25f),
    //     RectPos.height - InsetDouble}, 0.40f, 5, ColorBrightness(accentColor, -0.75f)
    //);
    BeginScissorMode(x, y, width + 2, height);
    DrawRectangleRounded(RectPos, 0.40f, 8, ColorBrightness(accentColor, -0.5f));
    EndScissorMode();

    float centerPos = x + (width / 2);
    GameMenu::mhDrawText(
        assets.redHatDisplayBlack,
        username.c_str(),
        { centerPos, (height / 4) + y },
        (height / 2),
        usernameColor,
        assets.sdfShader,
        CENTER
    );
    return toReturn;
}

bool encOS::OvershellButton(int slot, int x, std::string string) {
    Units &u = Units::getInstance();
    float OvershellLeftLoc = (u.wpct(0.125) + (u.winpct(0.25) * slot)) - u.winpct(0.1);
    return GuiButton(
        { OvershellLeftLoc,
          u.hpct(1.0f) - (u.winpct(0.03f) * (x + 1)),
          u.winpct(0.2f),
          u.winpct(0.03f) },
        string.c_str()
    );
    SETDEFAULTSTYLE();
}

bool encOS::OvershellCheckbox(int slot, int x, std::string string, bool initialVal) {
    Units &unit = Units::getInstance();
    float OvershellLeftLoc =
        (unit.wpct(0.125) + (unit.winpct(0.25) * slot)) - unit.winpct(0.1);
    float height = unit.winpct(0.03f);
    float widthNoHeight = unit.winpct(0.2f);
    Rectangle bounds = { OvershellLeftLoc + height,
                         unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
                         unit.winpct(0.2f) - height - height,
                         height };
    Rectangle confirmBounds = { OvershellLeftLoc + widthNoHeight,
                                unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
                                height,
                                height };
    Assets &assets = Assets::getInstance();

    if (GuiButton(
            { OvershellLeftLoc,
              unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
              widthNoHeight,
              height },
            string.c_str()
        )) {
        initialVal = !initialVal;
    }

    DrawRectanglePro(confirmBounds, { 0 }, 0, initialVal ? GREEN : RED);
    return initialVal;
}


bool encOS::OvershellSlider(
    int slot, int x, std::string string, float *value, float step, float min, float max
) {
    Units &unit = Units::getInstance();
    float OvershellLeftLoc =
        (unit.wpct(0.125) + (unit.winpct(0.25) * slot)) - unit.winpct(0.1);
    float height = unit.winpct(0.03f);
    float widthNoHeight = unit.winpct(0.2f) - height;
    Rectangle bounds = { OvershellLeftLoc + height,
                         unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
                         unit.winpct(0.2f) - height - height,
                         height };
    Rectangle confirmBounds = { OvershellLeftLoc + widthNoHeight,
                                unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
                                height,
                                height };
    Assets &assets = Assets::getInstance();

    GuiSlider(bounds, "", "", value, min, max);
    GuiButton(
        { OvershellLeftLoc,
          unit.hpct(1.0f) - (unit.winpct(0.03f) * (x + 1)),
          height,
          height },
        TextFormat("%1.1f", *value)
    );
    *value = (round(*value / step) * step);

    if (GuiButton(confirmBounds, "<")) {
        return true;
    };
    return false;
}
