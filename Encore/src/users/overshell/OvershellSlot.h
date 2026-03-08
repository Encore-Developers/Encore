//
// Created by deuce on 3/7/26.
//

#ifndef ENCORE_OVERSHELLSLOT_H
#define ENCORE_OVERSHELLSLOT_H
#include "users/player.h"

namespace Encore {
    class OvershellSlot {
    public:
        /// Will be null while a player is selecting their profile.
        Player* player;
        EncorePadID pad;

        bool open;

        OvershellSlot(Player* player) {
            this->player = player;
        }

        operator Player*() const {
            return this->player;
        }

        void DrawSlotTab(float overshellHeight);
        void ToggleOpen();
    };
}


#endif // ENCORE_OVERSHELLSLOT_H
