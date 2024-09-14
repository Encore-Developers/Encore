#pragma once
//
// Created by marie on 02/05/2024.
//

#include "GLFW/glfw3.h"
#include "song/songlist.h"
#include "assets.h"
#include "users/player.h"

enum Screens {
    MENU,
    SONG_SELECT,
    READY_UP,
    GAMEPLAY,
    RESULTS,
    SETTINGS,
    CALIBRATION,
    CHART_LOADING_SCREEN,
    SOUND_TEST,
    CACHE_LOADING_SCREEN
};

class GameMenu {
    template<typename CharT>
    struct Separators : public std::numpunct<CharT>
    {
        virtual std::string do_grouping()
        const
        {
            return "\003";
        }
    };



public:
    GameMenu() {};
    static Shader sdfShader() {
        std::filesystem::path assetsdir = GetApplicationDirectory();
        assetsdir /= "Assets";
        return LoadShader(0, (assetsdir / "fonts/sdf.fs").string().c_str());
    }

    static std::string scoreCommaFormatter(int value) {
        std::stringstream ss;
        ss.imbue(std::locale(std::cout.getloc(), new Separators <char>()));
        ss << std::fixed << value;
        return ss.str();
    }
    static void DrawTopOvershell(float TopOvershell);
    static void DrawBottomOvershell();
    static void DrawBottomBottomOvershell();
    static Texture2D LoadTextureFilter(const std::filesystem::path &texturePath);
    static Font LoadFontFilter(const std::filesystem::path &fontPath);
    static void mhDrawText(Font font, std::string, Vector2 pos, float fontSize, Color color);
    static void DrawFPS(int posX, int posY);
    static void DrawVersion();
    // drawing helper functions for other menus

    bool hehe = false;
    Screens currentScreen = CACHE_LOADING_SCREEN;
    bool songsLoaded{};
    bool streamsLoaded = false;
    bool streamsPaused = false;
    bool stringChosen = false;
    Texture2D AlbumArtBackground;
    bool albumArtLoaded = false;
    void loadMainMenu();
    inline void loadTitleScreen() {};

    void SwitchScreen(Screens screen);
    static void DrawAlbumArtBackground(Texture2D song);
    bool songChosen = false;


};

extern GameMenu TheGameMenu;
