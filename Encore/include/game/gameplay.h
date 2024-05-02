//
// Created by marie on 02/05/2024.
//

#ifndef ENCORE_GAMEPLAY_H
#define ENCORE_GAMEPLAY_H

#include <cstdlib>
#include "song/song.h"
#include "song/songlist.h"
#include "game/assets.h"
#include "game/settings.h"
#include "keybinds.h"
#include "game/utility.h"
#include "rhythmLogic.h"
#include "raygui.h"
#include "game/scenes.h"
#include <algorithm>


class Gameplay {
public:
    static std::string scoreCommaFormatter(int value);
    static void gameplay();

};


#endif //ENCORE_GAMEPLAY_H
