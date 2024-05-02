#pragma once
//
// Created by marie on 02/05/2024.
//

#include "GLFW/glfw3.h"
#include "song/songlist.h"
#include "assets.h"

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
public:
    Screens currentScreen;
    bool songsLoaded;

    void loadMenu(SongList songList, GLFWgamepadstatefun gamepadStateCallbackSetControls, Assets assets);
    inline void loadTitleScreen() {};

    void SwitchScreen(Screens screen);
};


