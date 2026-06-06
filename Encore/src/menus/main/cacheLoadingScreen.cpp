//
// Created by marie on 03/09/2024.
//

#include "cacheLoadingScreen.h"

#include <filesystem>
#include <mutex>
#include <thread>

#include "MainMenu.h"
#include "raymath.h"
#include "../util/uiUnits.h"
#include "song/songlist.h"
#include "raygui.h"
#include "../MenuManager.h"
#include "menus/util/locale/Locale.h"
#include "settings/settings.h"
#include "song/cacheload.h"

std::vector<std::string> CacheSplash = {
    "Want a break from the cache?",
    "Make sure to join the Discord if you havent!",
    "Remember when you could make coffee during these loading screens?",
    "When's the next train?",
    "Sorry for holding up the party!",
    "You can skip this screen by pressing Alt-F4!",
    "You're gonna be here for a bit..."
};

void cacheLoadingScreen::Load() {

    SplashSel = GetRandomValue(0, CacheSplash.size() - 1);
}


void cacheLoadingScreen::Draw() {
    Units u = Units::getInstance();
    float diagonalLength = sqrtf(
        (float)(GetRenderWidth() * GetRenderWidth())
        + (float)(GetRenderHeight() * GetRenderHeight())
    );
    float RectXPos = GetRenderWidth() / 2;
    float RectYPos = diagonalLength / 2;

    BeginShaderMode(ASSET(bgShader));
    DrawTexturePro(
        ASSET(bgLoadingScreen),
        Rectangle { 0, 0, (float)ASSET(bgLoadingScreen).width, (float)ASSET(bgLoadingScreen).width },
        Rectangle { RectXPos, -RectYPos * 2, diagonalLength * 2, diagonalLength * 2 },
        { 0, 0 },
        45,
        WHITE
    );
    EndShaderMode();

    GameMenu::DrawTopOvershell(0.15f);
    DrawRectangle(
        0,
        u.hpct(0.15f),
        Remap(CurrentChartNumber, 0, MaxChartsToLoad, 0, GetRenderWidth()),
        u.hinpct(0.01f),
        MAGENTA
    );

    Encore::Text::lDrawText(
        ASSET(redHatDisplayBlack),
        "cacheLoading.header",
        { u.LeftSide, u.hpct(0.05f) },
        u.hinpct(0.125f),
        WHITE,
        LEFT
    );
    float RubikFontSize = u.hinpct(0.05f);
    int loaded = CurrentChartNumber;
    int toLoad = MaxChartsToLoad;
    std::string songs = TextFormat("%d/%d", loaded, toLoad);
    std::string LoadingText = LOCALIZE_FMT("cacheLoading.loading", loaded, toLoad);
    Encore::Text::DrawText(
        ASSET(rubikBold),
        LoadingText,
        { u.RightSide, u.hpct(0.085f) },
        RubikFontSize,
        LIGHTGRAY,
        RIGHT
    );
    GameMenu::DrawBottomOvershell();

    Rectangle LogoRect = { u.LeftSide + u.hinpct(0.075f),
                           GetRenderHeight() - u.hpct(0.14f) + u.hinpct(0.07f),
                           u.hinpct(0.14f),
                           u.hinpct(0.14f) };

    auto logo = ASSETPTR(faviconTex);
    DrawTexturePro(
        *logo,
        { 0, 0, (float)logo->width, (float)logo->height },
        LogoRect,
        { u.hinpct(0.07f), u.hinpct(0.07f) },
        0,
        WHITE
    );
    Encore::Text::DrawText(
        ASSET(josefinSansItalic),
        CacheSplash[SplashSel],
        { u.LeftSide + u.hinpct(0.16),
          GetRenderHeight() - u.hpct(0.14f) + u.hinpct(0.055f) },
        RubikFontSize / 1.5f,
        WHITE,
        LEFT
    );
    if (!CacheLoad::started) {
        CacheLoad::StartLoad();
    }
    if (CacheLoad::finished) {
        TheMenuManager.CreateAndSwitchMenu<MainMenu>();
    }
}

cacheLoadingScreen::~cacheLoadingScreen() {}
cacheLoadingScreen::cacheLoadingScreen() {}
