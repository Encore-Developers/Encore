#pragma once
//
// Created by marie on 02/05/2024.
//

#include "GLFW/glfw3.h"
#include "../song/songlist.h"
#include "../assets.h"
#include "../users/player.h"

enum Screens {
    MENU,
    SONG_SELECT,
    READY_UP,
    GAMEPLAY,
    RESULTS,
    SETTINGS,
    CALIBRATION,
    CHART_LOADING_SCREEN,
    SOUND_TEST
};

class GameMenu {
private:


    template<typename CharT>
    struct Separators : public std::numpunct<CharT>
    {
        virtual std::string do_grouping()
        const
        {
            return "\003";
        }
    };

    std::string scoreCommaFormatter(int value) {
        std::stringstream ss;
        ss.imbue(std::locale(std::cout.getloc(), new Separators <char>()));
        ss << std::fixed << value;
        return ss.str();
    }
    void renderPlayerResults(Player player, Song song, int playerNum);
    void renderStars(PlayerGameplayStats* player, float xPos, float yPos, float scale, bool left);
    void renderStars(BandGameplayStats* player, float xPos, float yPos, float scale, bool left);
public:
    GameMenu() {}

    void DrawTopOvershell(float TopOvershell);
    void DrawBottomOvershell();
    void DrawBottomBottomOvershell();

    void DrawFPS(int posX, int posY);
    bool hehe = false;
    Song ChosenSong;
    int ChosenSongInt;
    Screens currentScreen;
    bool songsLoaded{};
    bool streamsLoaded = false;
    bool streamsPaused = false;
    bool stringChosen = false;
    Texture2D AlbumArtBackground;
    bool albumArtLoaded = false;
    void showResults();
    void loadMainMenu();
    inline void loadTitleScreen() {};

    void SwitchScreen(Screens screen);
    void DrawAlbumArtBackground(Texture2D song);
    bool songChosen = false;

    void DrawVersion();
};

extern GameMenu TheGameMenu;
