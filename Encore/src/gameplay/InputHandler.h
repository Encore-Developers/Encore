//
// Created by marie on 15/09/2024.
//

#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "users/player.h"

class InputHandler {
    static int calculatePressedMask(PlayerGameplayStats *stats);
    static bool
    isNoteMatch(const Note &curNote, int pressedMask, PlayerGameplayStats *stats);
    void CheckPlasticInputs(
        Player *player, int lane, int action, float eventTime, PlayerGameplayStats *stats
    );
public:
    void handleInputs(Player *player, int lane, int action);
};



#endif //INPUTHANDLER_H
