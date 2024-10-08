//
// Created by marie on 03/09/2024.
//

#include "cacheLoadingScreen.h"

#include <filesystem>
#include <thread>

#include "gameMenu.h"
#include "raymath.h"
#include "settings-old.h"
#include "uiUnits.h"
#include "song/songlist.h"
#include "raygui.h"

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
    std::filesystem::path assetsdir = GetApplicationDirectory();
    assetsdir /= "Assets";
    RedHatDisplay = GameMenu::LoadFontFilter(assetsdir / "fonts/RedHatDisplay-Black.ttf");
    RubikBold = GameMenu::LoadFontFilter(assetsdir / "fonts/Rubik-Bold.ttf");
    JosefinSansItalic =
        GameMenu::LoadFontFilter(assetsdir / "fonts/JosefinSans-Italic.ttf");
    encoreLogo = GameMenu::LoadTextureFilter(assetsdir / "encore_favicon-NEW.png");
    SplashSel = GetRandomValue(0, CacheSplash.size() - 1);
    sdfShader = LoadShader(0, (assetsdir / "fonts/sdf.fs").string().c_str());
}

// todo(3drosalia): make another class for drawing these things without having to uh.
// implement it in every menu class
bool finished = false;
bool started = false;

void LoadCache() {
    SongList &list = SongList::getInstance();
    SettingsOld &settings = SettingsOld::getInstance();
    list.LoadCache(settings.songPaths);
    finished = true;
}

void cacheLoadingScreen::Draw() {
    Units u = Units::getInstance();
    GameMenu::DrawTopOvershell(0.15f);
    // float logoHeight = u.hinpct(0.145f);
    // float logoWidth = Remap(encoreLogo.height, 0, encoreLogo.width / 4.25, 0,
    // u.winpct(0.5f)); Rectangle LogoRect = { u.RightSide - u.winpct(0.01f) - logoWidth,
    // u.hpct(0.035f), logoWidth, logoHeight}; DrawTexturePro(encoreLogo,
    // {0,0,(float)encoreLogo.width,(float)encoreLogo.height}, LogoRect, {0,0}, 0, WHITE);
    DrawRectangle(
        0,
        u.hpct(0.15f),
        Remap(CurrentChartNumber, 0, MaxChartsToLoad, 0, GetScreenWidth()),
        u.hinpct(0.01f),
        MAGENTA
    );

    GameMenu::mhDrawText(
        RedHatDisplay,
        "LOADING CACHE",
        { u.LeftSide, u.hpct(0.05f) },
        u.hinpct(0.125f),
        WHITE,
        sdfShader,
        LEFT
    );
    float RubikFontSize = u.hinpct(0.05f);
    int loaded = CurrentChartNumber;
    int toLoad = MaxChartsToLoad;
    std::string LoadingText = TextFormat("%d/%d songs loaded", loaded, toLoad);
    GameMenu::mhDrawText(
        RubikBold,
        LoadingText,
        { u.RightSide, u.hpct(0.085f) },
        RubikFontSize,
        LIGHTGRAY,
        sdfShader,
        RIGHT
    );
    GameMenu::DrawBottomOvershell();

    Rectangle LogoRect = { u.LeftSide + u.hinpct(0.075f),
                           GetScreenHeight() - u.hpct(0.14f) + u.hinpct(0.07f),
                           u.hinpct(0.14f),
                           u.hinpct(0.14f) };
    DrawTexturePro(
        encoreLogo,
        { 0, 0, (float)encoreLogo.width, (float)encoreLogo.height },
        LogoRect,
        { u.hinpct(0.07f), u.hinpct(0.07f) },
        0,
        WHITE
    );
    GameMenu::mhDrawText(
        JosefinSansItalic,
        CacheSplash[SplashSel],
        { u.LeftSide + u.hinpct(0.16),
          GetScreenHeight() - u.hpct(0.14f) + u.hinpct(0.055f) },
        RubikFontSize / 1.5f,
        WHITE,
        sdfShader,
        LEFT
    );
    if (!started) {
        started = true;
        std::thread CacheLoader(LoadCache);
        CacheLoader.detach();
        TheGameMenu.songsLoaded = true;
    }
    if (finished)
        TheGameMenu.SwitchScreen(MENU);
}

cacheLoadingScreen::~cacheLoadingScreen() {}
cacheLoadingScreen::cacheLoadingScreen() {}
