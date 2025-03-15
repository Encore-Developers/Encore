#pragma once
//
// Created by marie on 02/05/2024.
//

#include "GLFW/glfw3.h"
#include "song/songlist.h"
#include "assets.h"
#include "users/player.h"
#include "OvershellMenu.h"

enum TextAlign {
    LEFT,
    CENTER,
    RIGHT
};

template <typename CharT>
struct Separators : public std::numpunct<CharT> {
    virtual std::string do_grouping() const { return "\003"; }
};

namespace GameMenu {

    inline std::string scoreCommaFormatter(int value) {
        std::stringstream ss;
        ss.imbue(std::locale(std::cout.getloc(), new Separators<char>()));
        ss << std::fixed << value;
        return ss.str();
    }
    void DrawTopOvershell(float TopOvershell);
    void DrawBottomOvershell();
    Texture2D LoadTextureFilter(const std::filesystem::path &texturePath);
    Font LoadFontFilter(const std::filesystem::path &fontPath);
    void mhDrawText(
        Font font,
        std::string,
        Vector2 pos,
        float fontSize,
        Color color,
        Shader sdfShader,
        int align
    );
    void DrawFPS(int posX, int posY);
    void DrawVersion();

    void DrawAlbumArtBackground(Texture2D song);
}

class MainMenu : public OvershellMenu {
    
    void ChooseSplashText(std::filesystem::path directory);
    void PickRandomMenuSong();
    void AttractScreen();
    void MainMenuScreen();
public:
    // MainMenu() {};
    // drawing helper functions for other menus

    bool hehe = false;
    bool shouldBreak = false;

    MainMenu() {}
    ~MainMenu() {}
    void Draw();
    void Load();
    void KeyboardInputCallback(int key, int scancode, int action, int mods);
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state);
    std::string SplashString;
    bool songsLoaded = false;
    bool streamsLoaded = false;
    bool streamsPaused = false;
    bool stringChosen = false;
    Texture2D AlbumArtBackground;
    bool albumArtLoaded = false;
    void loadMainMenu();
    inline void loadTitleScreen() {};

    bool songChosen = false;
};
