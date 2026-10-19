#include "GameMenu.h"

#include "TextDisplay.h"
#include "raylib.h"
#include "uiUnits.h"
#include "song/ArtLoader.h"

std::string menuCommitHash = GIT_COMMIT_HASH;
std::string menuVersion = ENCORE_VERSION;
std::string gitBranch = GIT_BRANCH;


void GameMenu::DrawTopOvershell(float TopOvershell) {

    float OvershellBottom = Units::hpct(TopOvershell);
    float OvershellHeight = Units::hinpct(TopOvershell);
    float TopBarPos = Units::hpct(0);
    float TopBarHeight = Units::hinpct(0.04);

    DrawRectangleGradientV(
        0,
        OvershellBottom - 2,
        GetRenderWidth(),
        Units::hinpct(0.025f),
        Color { 0, 0, 0, 128 },
        Color { 0, 0, 0, 0 }
    );
    DrawRectangleGradientV(
        0,
        TopBarPos - Units::hinpct(0.025f),
        GetRenderWidth(),
        Units::hinpct(0.025f),
        Color { 0, 0, 0, 0 },
        Color { 0, 0, 0, 128 }
    );
    DrawRectangle(0, TopBarPos, (int)GetRenderWidth(), OvershellHeight, WHITE);
    DrawRectangleGradientV(
        0,
        TopBarPos,
        GetRenderWidth(),
        TopBarHeight,
        GetColor(0x332033FF),
        GetColor(0x271827FF)
    );
    DrawRectangleGradientV(
        0,
        TopBarPos + TopBarHeight,
        GetRenderWidth(),
        OvershellHeight - TopBarHeight - Units::hinpct(0.005f),
        Color { 32, 32, 51, 255 },
        Color { 24, 24, 39, 255 }
    );
    DrawRectangleGradientV(
        0,
        TopBarPos + TopBarHeight,
        GetRenderWidth(),
        Units::hinpct(0.0075f),
        Color { 0, 0, 0, 64 },
        Color { 0, 0, 0, 0 }
    );
    DrawRectangleGradientV(
        0,
        TopBarPos,
        GetRenderWidth(),
        Units::hinpct(0.005f),
        Color { 0, 0, 0, 64 },
        Color { 0, 0, 0, 0 }
    );
    //DrawRectangle(
    //    0,
    //    0,
    //    (int)GetRenderWidth(),
    //    Units::hpct(TopOvershell) - Units::hinpct(0.005f),
    //   ColorBrightness(GetColor(0x181827FF), -0.25f)
    // );
}

void GameMenu::DrawBottomOvershell() {
    float BottomOvershell = GetRenderHeight() - Units::hpct(0.18f);
    DrawRectangle(
        0, BottomOvershell, (float)(GetRenderWidth()), (float)GetRenderHeight(), WHITE
    );
    DrawRectangle(
        0,
        BottomOvershell + Units::hinpct(0.005f),
        (float)(GetRenderWidth()),
        (float)GetRenderHeight(),
        ColorBrightness(GetColor(0x181827FF), -0.5f)
    );
}

// should be reduced to just PlayerSongStats (instead of Player) eventually

// todo: replace player with band stats

void GameMenu::DrawAlbumArtBackground() {
    if (!TheArtLoader.loadedArtBlur) {
        return;
    }
    float diagonalLength = sqrtf(
        (float)(GetRenderWidth() * GetRenderWidth())
        + (float)(GetRenderHeight() * GetRenderHeight())
    );
    float RectXPos = GetRenderWidth() / 2;
    float RectYPos = diagonalLength / 2;

    BeginShaderMode(ASSET(bgShader));
    DrawTexturePro(
        *TheArtLoader.loadedArtBlur,
        Rectangle { 0, 0, (float)TheArtLoader.loadedArtBlur->GetTexture().width, (float)TheArtLoader.loadedArtBlur->GetTexture().width },
        Rectangle { RectXPos, -RectYPos * 2, diagonalLength * 2, diagonalLength * 2 },
        { 0, 0 },
        45,
        WHITE
    );
    EndShaderMode();
};


void GameMenu::DrawTopBarText(bool ShowFPS, bool ShowLogo) {
    float Size = Units::hinpct(0.025f);
    float Top = Units::hpct(00);
    float Offset = 0;

    if (ShowLogo) {
        Encore::TextDisplay logo;
        logo.Pos(Units::wpct(0.0025f), Top - Units::hinpct(0.003))
            .Size(Size*1.75)
            .Col(WHITE)
            .Fnt(ASSET(rubikExtraBold))
            .DrawText("encore");
        Offset = logo.TextWidth("encore");
    }

    Encore::TextDisplay vers;
    std::string VersionText = menuVersion + "-" + gitBranch + ":" + menuCommitHash.substr(0, 6);
    vers.Pos(Units::wpct(0.0025f) + (Offset * 1.125), Top + (Units::hinpct(0.0225) - (Size /2)))
        .Size(Size)
        .Col(GetColor(0x807F7FFF))
        .Fnt(ASSET(josefinSansBold))
        .DrawText(VersionText);

    if (!ShowFPS) return;

    vers.AddX(vers.TextWidth(VersionText) * 1.125)
        .Col(LIME)
        .DrawText(std::format("FPS: {}", GetFPS()));
};