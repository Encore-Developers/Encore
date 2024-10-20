//
// Created by marie on 15/09/2024.
//

#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "users/playerManager.h"

enum EventLanes {
    OVERDRIVE_ACT = -1,
    LANE_1 = 0,
    LANE_2 = 1,
    LANE_3 = 2,
    LANE_4 = 3,
    LANE_5 = 4,
    STRUM = 8008135,
};

class InputHandler {
    static int calculatePressedMask(PlayerGameplayStats *stats);
    static bool
    isNoteMatch(const Note &curNote, int pressedMask, PlayerGameplayStats *stats);
    void CheckPlasticInputs(
        Player *player, int lane, int action, float eventTime, PlayerGameplayStats *stats
    );
public:
    void handleInputs(Player *player, int lane, int action);
    void CheckPadInputs(
        Player *player, int lane, int action, double eventTime, PlayerGameplayStats *stats
    );
};



#endif //INPUTHANDLER_H
