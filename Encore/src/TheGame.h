//
// Created by maria on 10/06/2026.
//

#ifndef ENCORE_THEGAME_H
#define ENCORE_THEGAME_H
#include "util/enclog.h"


struct TheGame
{
    int Run(int argc, char *argv[]);
    TheGame();
    ~TheGame();
    bool shouldQuit = false;
};

extern TheGame game;
#endif //ENCORE_THEGAME_H