//
// Created by maria on 14/01/2026.
//

#ifndef ENCORE_GUITARTRACK_H
#define ENCORE_GUITARTRACK_H
#include "BaseTrack.h"
#include "gameplay/enctime.h"

namespace Encore {
    class GuitarTrack : public BaseTrack
    {
    public:
        explicit GuitarTrack(Player &player_)
            : BaseTrack(player_) {
        };
        void DrawNotes() override;
        void DrawStrikeline() override;
        void DrawBeatlines() override;
        std::span<Beatline> BeatlinePool;
    };
}

#endif //ENCORE_GUITARTRACK_H