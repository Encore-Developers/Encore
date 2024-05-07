#pragma once
//
// Created by marie on 02/05/2024.
//

#include "GLFW/glfw3.h"
#include "song/songlist.h"
#include "assets.h"
#include "player.h"

enum Screens {
    MENU,
    SONG_SELECT,
    INSTRUMENT_SELECT,
    DIFFICULTY_SELECT,
    GAMEPLAY,
    RESULTS,
    SETTINGS
};

class Menu {
private:

    bool stringChosen = false;

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

    void renderStars(Player player, float xPos, float yPos, Assets assets);
public:
    static void DrawTopOvershell(float TopOvershell);
    void DrawBottomOvershell();
    void DrawBottomBottomOvershell();

    Screens currentScreen;
    bool songsLoaded{};
    void showResults(const Player& player, Assets assets);
    void loadMenu(SongList songList, GLFWgamepadstatefun gamepadStateCallbackSetControls, Assets assets);
    inline void loadTitleScreen() {};

    void SwitchScreen(Screens screen);
};


