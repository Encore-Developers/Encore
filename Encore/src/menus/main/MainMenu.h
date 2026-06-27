#pragma once
//
// Created by marie on 02/05/2024.
//

#include "song/songlist.h"
#include "assets.h"
#include "users/player.h"
#include "../overshell/OvershellMenu.h"
#include "menus/util/locale/Locale.h"
#include "menus/util/ButtonActionRegistry.h"
#include "menus/util/TextDisplay.h"
#include "song/ArtLoader.h"


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
    static bool FirstMainMenuBoot = true;

    static bool streamsLoaded = false;
    void DrawFPS(int posX, int posY);
    void DrawTopBarText(bool ShowFPS = false, bool ShowLogo = true);

    void DrawAlbumArtBackground();
}

class MainMenu : public OvershellMenu {
    
    void ChooseSplashText(std::filesystem::path directory);
    void DrawMiniMTVOverlay(unsigned char alpha, Vector2 pos);
    void AttractScreen();
    void GotoSongSelect();
    void MainMenuScreen();
public:
    static int logoInt;
    // MainMenu() {};
    // drawing helper functions for other menus

    bool hehe = false;
    bool shouldBreak = false;
    Encore::ButtonActionRegistry buttReg;
    MainMenu() {}
    ~MainMenu() {}
    void Draw();
    void Load();
    void KeyboardInputCallback(SDL_KeyboardEvent* event);
    void ControllerInputCallback(Encore::ControllerEvent event);
    std::string SplashString;
    float TitleAnimTimer = 1;
    int ControllerSelected = 0;
    bool songsLoaded = false;
    bool streamsPaused = false;
    bool stringChosen = false;
    bool albumArtLoaded = false;
    void loadMainMenu();
    inline void loadTitleScreen() {};

    bool songChosen = false;
};


