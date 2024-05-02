//
// Created by marie on 02/05/2024.
//
#include <string>
#include "lerp.h"

#ifndef ENCORE_SCENES_H
#define ENCORE_SCENES_H

Lerp lerp;

enum Screens {
    SONG_LOADING_SCREEN,
    MENU,
    SONG_SELECT,
    INSTRUMENT_SELECT,
    DIFFICULTY_SELECT,
    GAMEPLAY,
    RESULTS,
    SETTINGS
};

Lerp lerpCtrl = Lerp();
void updateStates();

LerpState getState(std::string key);

class Scenes {
public:

    static void SwitchScreen(Screens screen);

    static int currentScreen;
};


#endif //ENCORE_SCENES_H
