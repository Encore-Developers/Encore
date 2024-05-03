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
    void renderStars(Player player, float xPos, float yPos);
public:
    Screens currentScreen;
    bool songsLoaded;
    void showResults(Player player);
    void loadMenu(SongList songList, GLFWgamepadstatefun gamepadStateCallbackSetControls, Assets assets);
    inline void loadTitleScreen() {};

    void SwitchScreen(Screens screen);
};


