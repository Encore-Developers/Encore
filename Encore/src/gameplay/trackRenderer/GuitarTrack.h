//
// Created by maria on 14/01/2026.
//

#ifndef ENCORE_GUITARTRACK_H
#define ENCORE_GUITARTRACK_H
#include "Track.h"
#include "gameplay/enctime.h"

#ifdef nuhuh

namespace Encore {
    class GuitarTrack : public Track
    {
    public:
        explicit GuitarTrack(Player &player_)
            : Track(player_) {
        };
        void DrawNotes() override;
        void DrawStrikeline() override;
        void DrawBeatlines() override;
        std::span<Beatline> BeatlinePool;
    };
}
#endif

#endif //ENCORE_GUITARTRACK_H