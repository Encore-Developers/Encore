//
// Created by maria on 23/02/2026.
//

#ifndef ENCORE_KICKTRACKSLOT_H
#define ENCORE_KICKTRACKSLOT_H
#include "TrackSlot.h"

namespace Encore {
    class KickTrackSlot : public TrackSlot
    {
    public:
        KickTrackSlot(Track *track, float xPos, float width, ColorSlot colorSlot) : TrackSlot(track, xPos, width, colorSlot) {};

        float animTimer = 1;
        virtual void DrawNote(RhythmEngine::EncNote *note);
        virtual void DrawSustainTail(double startTime, double endTime) override {};
        virtual void DrawSmasher(bool held);
        virtual void AnimateHit() {
            animTimer = 0;
        };
    };
}

#endif //ENCORE_KICKTRACKSLOT_H