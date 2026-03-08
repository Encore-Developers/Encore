//
// Created by deuce on 3/7/26.
//

#ifndef ENCORE_OVERSHELLSLOT_H
#define ENCORE_OVERSHELLSLOT_H
#include "users/player.h"

namespace Encore {
    class OvershellSlot {
    public:
        Player* player;

        OvershellSlot(Player* player) {
            this->player = player;
        }

        operator Player*() const {
            return this->player;
        }
    };
}


#endif // ENCORE_OVERSHELLSLOT_H
