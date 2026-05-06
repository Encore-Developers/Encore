//
// Created by maria on 23/02/2026.
//

#ifndef ENCORE_OPENTRACKSLOT_H
#define ENCORE_OPENTRACKSLOT_H
#include "TrackSlot.h"


namespace Encore {

    class OpenTrackSlot : public TrackSlot {
    public:
        OpenTrackSlot(Track *track, float xPos, float width, ColorSlot colorSlot) : TrackSlot(track, xPos, width, 1, colorSlot) {
            openHitAnim = true;
        };

        virtual void DrawNote(RhythmEngine::EncNote *note, bool missed) override;
        virtual void DrawSustainTail(double startTime, double endTime, Color color) override;
        virtual void DrawSmasher(bool held) override {};
        virtual void AnimateHit(bool perfect, Color color) override {};
    };

}


#endif //ENCORE_OPENTRACKSLOT_H